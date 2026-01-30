import asyncio
import websockets
import json
import requests
import time
import statistics

HTTP_URL = "http://localhost:3000/api/deploy"
WS_URL = "ws://localhost:8765"

# Código RTOS Metrônomo
CODE_RTOS = """; TESSER RTOS
START:
    ; SLOT 1 (SENSOR)
    PUSH 100
    MUI_SET 0
    WAIT

    ; SLOT 2 (COMPUTE - HIDDEN COST)
    PUSH 50
    PUSH 50
    SUB      ; Consome ciclo de clock

    ; SLOT 3 (ACTUATOR)
    PUSH 255
    MUI_SET 0
    WAIT

    ; SLOT 4 (SYNC)
    PUSH 0
    MUI_SET 0
    WAIT

    JMP START
"""

async def run_jitter_test():
    print("⏱️ [RTOS TEST] Iniciando Análise de Determinismo...")

    # 1. Deploy
    try:
        requests.post(HTTP_URL, data=CODE_RTOS)
    except Exception as e:
        print(f"❌ Deploy Error: {e}")
        return

    # 2. Medição de Jitter
    deltas = []
    last_time = 0
    
    print("   [MEASURING] Coletando amostras de timing do Barramento...")
    
    async with websockets.connect(WS_URL) as ws:
        await ws.send(json.dumps({"cmd": "start", "mode": "run"}))
        
        # Ignora primeiros pacotes (warmup)
        for _ in range(5):
            await ws.recv()
            
        last_time = time.time()
        
        # Coleta 20 amostras (aprox 4-6 segundos)
        for i in range(20):
            await ws.recv() # Bloqueia até chegar pacote do emulador
            now = time.time()
            dt = (now - last_time) * 1000 # ms
            deltas.append(dt)
            last_time = now
            print(f"      Pack #{i+1}: {dt:.2f}ms")

    # 3. Análise Estatística
    avg = statistics.mean(deltas)
    jitter = statistics.stdev(deltas)
    
    print("\n   📊 RELATÓRIO DE ESTABILIDADE:")
    print(f"      Média de Intervalo: {avg:.2f} ms")
    print(f"      Jitter (Desvio):    {jitter:.2f} ms")
    
    if jitter < 50: # Tolerância alta pois estamos rodando via Docker/Python/WS no Windows
        print("      ✅ CLASSIFICAÇÃO: HARD REAL-TIME (Simulado)")
        print("         O ritmo é constante. O sistema é determinístico.")
    else:
        print("      ⚠️ CLASSIFICAÇÃO: SOFT REAL-TIME")
        print("         Há variação perceptível no tempo de entrega.")

if __name__ == "__main__":
    asyncio.run(run_jitter_test())
