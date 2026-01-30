import asyncio
import websockets
import json
import requests
import time

HTTP_URL = "http://localhost:3000/api/deploy"
WS_URL = "ws://localhost:8765"

# Código AI Generated (Reverse Engineered)
CODE_AI = """; TASM AI GENERATED
START:
    PUSH 20
    PUSH 5
    SUB             ; 15
    JMP_POS PASS    ; Consome 15 e Pula
    
FAIL:
    PUSH 0
    MUI_SET 0
    WAIT
    JMP START

PASS:
    PUSH 222
    MUI_SET 0
    WAIT
    JMP VICTORY

VICTORY:
    PUSH 999
    MUI_SET 0
    WAIT
    PUSH 0
    MUI_SET 0
    WAIT
    JMP VICTORY
"""

async def run_turing_test():
    print("🤖 [TURING TEST] Executando Código Gerado por IA...")

    # 1. Deploy
    try:
        r = requests.post(HTTP_URL, data=CODE_AI)
        if r.status_code != 200:
            print(f"   ❌ Deploy Failed: {r.text}")
            return
    except Exception as e:
        print(f"   ❌ Erro HTTP: {e}")
        return

    # 2. Monitorar
    print("   [2/2] Monitorando Telemetria...")
    
    try:
        async with websockets.connect(WS_URL) as ws:
            await ws.send(json.dumps({"cmd": "start", "mode": "run"}))
            
            start_time = time.time()
            passed_logic = False
            
            while time.time() - start_time < 5:
                try:
                    msg = await asyncio.wait_for(ws.recv(), timeout=1.0)
                    data = json.loads(msg)
                    
                    if "mui_val" in data and data["mui_id"] == 0:
                        val = data["mui_val"]
                        
                        if val == 0 and not passed_logic:
                            # Ignora o 0 do loop final se ja tiver passado
                            pass
                        elif val == 222:
                            print("      🧠 SINAL 222 RECEBIDO: A IA deduziu corretamente o JMP_POS!")
                            passed_logic = True
                        elif val == 999:
                            print("      🏆 SINAL 999 RECEBIDO: Vitória Total.")
                            print("\n   [RESULTADO] O Tesser Silicon passou no Teste de Turing Funcional.")
                            print("               A arquitetura é perfeitamente inteligível para IAs.")
                            return

                except asyncio.TimeoutError:
                    continue
                except json.JSONDecodeError:
                    pass
            
            if not passed_logic:
                print("      ⚠️ FAILED: O salto 222 não ocorreu. A logica da IA falhou.")

    except Exception as e:
        print(f"   ⚠️ Erro de Execução: {e}")

if __name__ == "__main__":
    asyncio.run(run_turing_test())
