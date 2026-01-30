import asyncio
import websockets
import subprocess
import os
import http.server
import socketserver
import threading
import sys
import json

# Configurações
HTTP_PORT = 3000
WS_PORT = 8765
EMULATOR_BIN = "./tesser_tower"
FIRMWARE_SRC = "stress.tasm"
FIRMWARE_HEX = "firmware.hex"
DEBUG_MAP = "debug_map.json"

def build_system():
    print("🔨 [BUILD] Rebuild inicial...")
    # 1. Compilar C (apenas se necessário, mas o make lida com isso)
    if os.path.exists("Makefile"):
        try:
           subprocess.run(["make", "all"], check=True, stdout=subprocess.DEVNULL)
        except:
           pass 

class IDEHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Cache-Control', 'no-store')
        super().end_headers()

    def do_GET(self):
        # API: Get current source code
        if self.path == '/api/source':
            if os.path.exists(FIRMWARE_SRC):
                with open(FIRMWARE_SRC, 'rb') as f:
                    content = f.read()
                self.send_response(200)
                self.send_header('Content-Type', 'text/plain')
                self.end_headers()
                self.wfile.write(content)
            else:
                self.send_response(404)
                self.end_headers()
                self.wfile.write(b"; New Program")
        else:
            super().do_GET()

    def do_POST(self):
        # API: Receive code, Save, Assemble (TASM)
        if self.path == '/api/deploy':
            content_length = int(self.headers['Content-Length'])
            code_data = self.rfile.read(content_length)
            
            # 1. Save Source
            with open(FIRMWARE_SRC, 'wb') as f:
                f.write(code_data)
            
            print(f"📝 [IDE] Código recebido ({len(code_data)} bytes). Compilando...")

            # 2. Run TASM
            try:
                # Run tasm.py and capture output
                result = subprocess.run(
                    ["python", "tasm.py", FIRMWARE_SRC, FIRMWARE_HEX], 
                    capture_output=True, 
                    text=True
                )
                
                if result.returncode == 0:
                    print("✅ [IDE] Compilação OK.")
                    self.send_response(200)
                    self.send_header('Content-Type', 'application/json')
                    self.end_headers()
                    self.wfile.write(json.dumps({"status": "ok", "logs": result.stdout}).encode())
                else:
                    print("❌ [IDE] Erro de Compilação.")
                    self.send_response(400) # Bad Request
                    self.send_header('Content-Type', 'application/json')
                    self.end_headers()
                    err_msg = result.stderr if result.stderr else result.stdout
                    self.wfile.write(json.dumps({"status": "error", "logs": err_msg}).encode())

            except Exception as e:
                self.send_response(500)
                self.end_headers()
                self.wfile.write(json.dumps({"status": "server_error", "logs": str(e)}).encode())
        else:
            self.send_response(404)
            self.end_headers()

async def telemetry_handler(websocket):
    print("🔌 [WS] Cliente conectado. Aguardando Handshake...")
    process = None
    
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
            try:
                while True:
                    line = await process.stdout.readline()
                    if not line: break
                    await websocket.send(line.decode().strip())
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
        # O pipe_stdout vai morrer quando o processo morrer.
        # O listen_ws vai morrer quando o websocket fechar.
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

async def main():
    build_system()

    http_thread = threading.Thread(target=start_http_server, daemon=True)
    http_thread.start()

    print(f"📡 [WS] Servidor na porta {WS_PORT}")
    async with websockets.serve(telemetry_handler, "0.0.0.0", WS_PORT):
        await asyncio.Future()

if __name__ == "__main__":
    if sys.platform == 'win32':
        asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())
    asyncio.run(main())
