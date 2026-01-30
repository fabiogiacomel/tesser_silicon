import http.server
import socketserver
import subprocess
import os
import json
import asyncio
import websockets
import threading

PORT = 3000
HTTP_PORT = 3000
WS_PORT = 8765
EMULATOR_BIN = "./tesser_tower"

# Configuração de Gravação de Dataset
RECORDING = True
DATASET_FILE = "tesser_data.jsonl"

class IDEHandler(http.server.SimpleHTTPRequestHandler):
    def do_POST(self):
        if self.path == '/api/deploy':
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            
            # 1. Save Code
            with open("firmware.tasm", "wb") as f:
                f.write(post_data)
            
            # 2. Compile
            print("🔨 [BUILD] Rebuild inicial...")
            try:
                # Run tasm.py to generate firmware.hex and debug_map.json
                res = subprocess.run(["python3", "tasm.py", "firmware.tasm"], capture_output=True, text=True)
                
                if res.returncode != 0:
                     self.send_response(400)
                     self.send_header('Content-type', 'application/json')
                     self.end_headers()
                     self.wfile.write(json.dumps({"status": "error", "logs": res.stderr}).encode())
                     return

                # Compile C Emulator
                subprocess.run(["make", "all"], check=True)
                
                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"status": "success", "logs": "Compilation OK"}).encode())
                
            except Exception as e:
                self.send_response(500)
                self.end_headers()
                self.wfile.write(str(e).encode())

        else:
            super().do_POST()

    def do_GET(self):
        if self.path == '/api/source':
            if os.path.exists("firmware.tasm"):
                self.send_response(200)
                self.end_headers()
                with open("firmware.tasm", "rb") as f:
                    self.wfile.write(f.read())
            else:
                self.send_response(404)
                self.end_headers()
        else:
            super().do_GET()

async def telemetry_handler(websocket):
    print("🔌 [WS] Cliente conectado. Aguardando Handshake...")
    process = None
    
    # Carrega Mapa de Debug (se existir)
    debug_map = None
    try:
        if os.path.exists("debug_map.json"):
            with open("debug_map.json", "r") as f:
                debug_map = json.load(f)
                print("🗺️ [WS] Debug Map carregado para Flight Recorder.")
    except Exception as e:
        print(f"⚠️ [WS] Falha ao carregar debug_map.json: {e}")

    # Contador de passos para o dataset
    step_count = 0

    try:
        # 1. Aguarda Handshake inicial
        msg = await websocket.recv()
        data = json.loads(msg)
        
        if data.get("cmd") != "start":
            print("❌ [WS] Handshake ignorado (esperava 'start').")
            return

        mode = data.get("mode", "run")
        print(f"🚀 [WS] Iniciando Emulação (Async). Modo: {mode.upper()}")
        
        args = [EMULATOR_BIN]
        if mode == "debug":
            args.append("--debug")

        # 2. Inicia Processo de forma 100% Async
        process = await asyncio.create_subprocess_exec(
            *args,
            stdout=asyncio.subprocess.PIPE,
            stdin=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE
        )

        # 3. Define Tarefas de I/O
        async def pipe_stdout():
            nonlocal step_count
            try:
                while True:
                    line_bytes = await process.stdout.readline()
                    if not line_bytes: break
                    
                    line_str = line_bytes.decode().strip()
                    if not line_str: continue

                    # --- FLIGHT RECORDER LOGIC START ---
                    if RECORDING and debug_map:
                        try:
                            # Tenta parsear para capturar dados
                            state = json.loads(line_str)
                            if "pc" in state:
                                pc = state["pc"]
                                # Lookup no mapa
                                line_idx = debug_map.get("address_map", {}).get(str(pc))
                                
                                source_line = ""
                                opcode_extracted = ""
                                if line_idx is not None:
                                    try:
                                        source_line = debug_map["source_code"][line_idx].strip()
                                        # Extrai primeira palavra como Opcode
                                        parts = source_line.split()
                                        if parts:
                                            opcode_extracted = parts[0].replace(':', '')
                                    except: pass
                                
                                stack = state.get("stack", [])[:state.get("sp", 0)]
                                stack_top = stack[-1] if stack else None

                                # Cria objeto de treino rico
                                training_entry = {
                                    "step": step_count,
                                    "pc": pc,
                                    "opcode": opcode_extracted,
                                    "source": source_line,
                                    "stack_top": stack_top,
                                    "full_stack": stack,
                                    "mui_val": state.get("mui_val", 0)
                                }
                                
                                # Grava no arquivo (Append Mode)
                                with open(DATASET_FILE, "a") as df:
                                    df.write(json.dumps(training_entry) + "\n")
                                
                                step_count += 1
                        except json.JSONDecodeError:
                            pass
                        except Exception as rec_err:
                            print(f"⚠️ [RECORDER] Erro: {rec_err}")
                    # --- FLIGHT RECORDER LOGIC END ---

                    await websocket.send(line_str)
            except Exception as e:
                print(f"Stdout Pipe Error: {e}")

        async def listen_ws():
            try:
                async for message in websocket:
                    cmd_data = json.loads(message)
                    if cmd_data.get("cmd") == "step":
                        if process.stdin:
                            process.stdin.write(b'\n')
                            await process.stdin.drain()
                    elif cmd_data.get("cmd") == "stop":
                        process.terminate()
            except Exception as e:
                print(f"WS Listener Error: {e}")

        # 4. Executa em paralelo até desconexão
        await asyncio.gather(pipe_stdout(), listen_ws(), return_exceptions=True)

    except websockets.exceptions.ConnectionClosed:
        print("⚠️ [WS] Conexão fechada.")
    except Exception as e:
        print(f"⚠️ [WS] Erro Geral: {e}")
    finally:
        if process:
            try: 
                process.kill()
            except: pass
        print("🔌 [WS] Sessão Finalizada.")

def start_http_server():
    with socketserver.TCPServer(("", HTTP_PORT), IDEHandler) as httpd:
        print(f"🌐 [HTTP] IDE disponível em http://localhost:{HTTP_PORT}/vd/index.html")
        httpd.serve_forever()

async def start_ws_server():
    print(f"📡 [WS] Servidor na porta {WS_PORT}")
    async with websockets.serve(telemetry_handler, "0.0.0.0", WS_PORT):
        await asyncio.Future()  # roda pra sempre

if __name__ == "__main__":
    # Inicia HTTP em thread separada
    http_thread = threading.Thread(target=start_http_server)
    http_thread.daemon = True
    http_thread.start()

    # Inicia WebSocket no loop principal
    asyncio.run(start_ws_server())
