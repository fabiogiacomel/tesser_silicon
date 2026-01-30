# Tesser Silicon v1.0 — Stack Machine Architecture

![Build Status](https://img.shields.io/badge/status-GOLD%20MASTER-ffd700.svg)
![Architecture](https://img.shields.io/badge/arch-STACK%20MACHINE-blue.svg)
![AI Ready](https://img.shields.io/badge/AI-DATASET%20GENERATOR-green.svg)

> "Uma máquina capaz de ensinar a sua própria física a uma Inteligência Artificial."

## 🌟 Visão Geral
O **Tesser Silicon** é um processador virtual de 16-bit baseado em pilha (Stack Machine), projetado para ser simples, determinístico e observável. 

O projeto alcançou o status de **Gold Master** ao demonstrar um ciclo completo de "Engenharia Reversa Cognitiva":
1.  **Hardware:** Executa instruções.
2.  **Telemetry:** Gera logs detalhados de estado (`.jsonl`).
3.  **AI:** Um LLM (Llama/GPT) lê os logs e *deduz* a arquitetura sem ver o código-fonte.
4.  **Loop:** A IA escreve código Assembly válido que o Hardware executa com sucesso.

---

## 🏗️ Arquitetura do Sistema

### 1. The Core (C / Verilog Spec)
* **Tipo:** LIFO Stack Machine (Last In, First Out).
* **Capacidade da Pilha:** 16 Slots (16-bit).
* **Memória de Instrução:** Harvard Architecture (Separada de dados).
* **Proteção:** Stack Overflow/Underflow Detection.

### 2. Instruction Set Architecture (ISA)
| Opcode | Mnemonic | Argumento | Descrição |
| :--- | :--- | :--- | :--- |
| `0x01` | `PUSH` | `val` (16b) | Empilha um valor imediato. |
| `0x02` | `MUI_SET`| `id` (8b) | Desempilha valor e envia para porta de I/O `id`. |
| `0x03` | `WAIT` | - | Pausa o ciclo de clock (Debug/Visualização). |
| `0x04` | `JMP` | `addr` (16b)| Salto incondicional. |
| `0x07` | `SUB` | - | `Pop A, Pop B -> Push (A - B)`. |
| `0x08` | `JMP_POS`| `addr` (16b)| `Pop A`. Se `A > 0`, salta. |

### 3. A Toolchain
* **Assembler (`tasm.py`):** Converte código Assembly `.tasm` para binário `.hex` e gera mapas de debug (`debug_map.json`).
* **Maestro (`server.py`):** Orquestrador em Python que gerencia a compilação, o emulador em C e o servidor WebSocket.
* **IDE Visual (`index.html`):** Interface Web com telemetria em tempo real, visualização de pilha e editor de código com *hot-reload*.

---

## 🚀 Como Executar (Docker)

O ambiente é "One-Click Deploy". Todo o ecossistema roda num container isolado.

```bash
# 1. Construir a Imagem
docker build -t tesser_env .

# 2. Rodar o Sistema (Mapeando a pasta atual para Hot-Reload)
docker run -it -p 3000:3000 -p 8765:8765 -v $(pwd):/app tesser_env

```

Acesse **`http://localhost:3000/vd/index.html`** para assumir o controle.

---

## 🤖 AI Dataset Generation

O sistema possui um modo "Flight Recorder" que grava cada micro-operação da CPU.

* **Arquivo:** `tesser_data.jsonl`
* **Formato:**
```json
{
  "step": 102,
  "opcode": "SUB",
  "stack_before": [100, 20, 5],
  "stack_after": [100, 15],
  "source_code": "SUB ; 20 - 5"
}

```


* **Aplicação:** Este dataset é ideal para Fine-Tuning de LLMs em tarefas de raciocínio algorítmico e compreensão de fluxo de dados.

---

## 🏆 Créditos

* **Arquitetura & Conceito:** O Comandante.
* **Implementação de Sistema:** Antigravity (AI Assistant).
* **Validação:** Teste de Turing de Hardware (Aprovado).

*v1.0 - Ampére, Paraná - 2026*
