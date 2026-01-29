# Tesser Silicon (v1.0 Gold Master)
**Processador Soft-Core de 16-bits para Robótica Bio-Inspirada**

> "Se o software é a mente, o silício é o sistema nervoso."

O **Tesser Silicon** é a implementação em hardware (RTL Verilog) da arquitetura *Tesser Stack Machine*. Projetado para ser sintetizado em FPGA, este processador executa nativamente o bytecode `.txe` (Tesser Executable), oferecendo controle determinístico e paralelo para atuadores robóticos, eliminando a latência de interpretadores de software.

---

## 🏗️ Especificações de Hardware

### Arquitetura
* **Tipo:** Máquina de Pilha (Stack Machine) de 16-bits.
* **Pipeline:** 3 Estágios (Fetch, Decode, Execute).
* **Clock:** Otimizado para 50MHz-100MHz (Configurável via `CYCLES_PER_MS`).
* **Memória de Programa:** ROM Interna (Carregável via `.hex`).
* **Memória de Dados:** Pilha de Hardware (LIFO) de 16 posições.

### Interface de Periféricos (MUI Bus)
O processador se comunica com o mundo externo através do barramento **MUI** (Menor Unidade Indivisível).

| Porta | Direção | Largura | Descrição |
| :--- | :--- | :--- | :--- |
| `clk` | Input | 1-bit | Clock do Sistema. |
| `reset` | Input | 1-bit | Reset Ativo Alto (Síncrono). |
| `mui_id` | Output | 8-bits | Endereço do Órgão (0-255). |
| `mui_val` | Output | 16-bits | Valor de Controle (0-1000). |
| `mui_we` | Output | 1-bit | **Write Enable:** Pulso para escrever no atuador. |
| `mui_rx_val`| Input | 16-bits | Valor lido de um sensor externo. |
| `mui_re` | Output | 1-bit | **Read Enable:** Pulso para ler de um sensor. |

---

## 📜 Instruction Set Architecture (ISA v1.0)

O Tesser Silicon suporta nativamente 8 Opcodes fundamentais para controle robótico.

| Opcode | Mnemônico | Argumentos | Descrição |
| :--- | :--- | :--- | :--- |
| `0x01` | `PUSH` | `[uint16]` | Empilha um valor imediato de 16 bits. |
| `0x02` | `MUI_SET` | `[uint8]` | Desempilha valor e escreve na porta `mui_id`. |
| `0x03` | `WAIT` | - | Desempilha `ms`. Pausa a CPU por `ms` milissegundos. |
| `0x04` | `JMP` | `[addr16]` | Salta incondicionalmente para o endereço. |
| `0x05` | `SMOOTH` | `[uint8]` | *(Reservado)* Em HW, atua como `MUI_SET` por enquanto. |
| `0x06` | `MUI_GET` | `[uint8]` | Lê sensor `mui_id` e empilha o valor. |
| `0x07` | `SUB` | - | `Pop A, Pop B -> Push (A - B)`. |
| `0x08` | `JMP_POS` | `[addr16]` | `Pop A`. Se `A > 0`, salta para endereço. |

---

## 📂 Estrutura do Repositório

```text
tesser_silicon/
├── src/
│   └── tesser_cpu.v      # O Núcleo do Processador (RTL)
├── tb/
│   └── tb_tesser.v       # Testbench de Simulação
├── sim/                  # Diretório de Build (Ignorado pelo git)
├── firmware.hex          # Código de Máquina (Gerado pelo Assembler)
├── pisca.tasm            # Código Fonte de Exemplo
├── docs/
│   ├── ARCHITECTURE.md   # Diagrama de Blocos e Mapa da CPU
│   └── IMPLEMENTATION_PLAN.md
├── firmware.hex          # Código de Máquina (Gerado pelo Assembler)
```

---

## 🚀 Guia de Desenvolvimento (Toolchain)

Este projeto utiliza ferramentas Open Source padrão da indústria.

### Pré-requisitos

* **Icarus Verilog:** Para compilação e simulação (`iverilog`, `vvp`).
* **GTKWave:** Para visualização de ondas (`gtkwave`).
* **Python 3:** Para o Assembler.

### Fluxo de Trabalho (Build & Run)

1. **Codificar (Assembly):**
Escreva seu comportamento em `pisca.tasm`.
```asm
START:
    PUSH 1000
    MUI_SET 0
    PUSH 500
    WAIT
    JMP START
```

2. **Montar (Assembler):**
Gere o binário `.hex` para a ROM do processador.
```bash
python tasm.py pisca.tasm
```

3. **Compilar Hardware (Verilog):**
Sintetize o design e o testbench.
```bash
iverilog -o sim/tesser.out -s tb_tesser src/tesser_cpu.v tb/tb_tesser.v
```

4. **Simular:**
Rode a simulação.
```bash
vvp sim/tesser.out
```

5. **Visualizar (Opcional):**
Inspecione os sinais elétricos.
```bash
gtkwave sim/tesser_wave.vcd
```

---

## ⏱️ Configuração de Clock (FPGA)

No arquivo `src/tesser_cpu.v`, ajuste o parâmetro `CYCLES_PER_MS` de acordo com o clock do seu FPGA alvo para garantir que o comando `WAIT` funcione em tempo real.

* **Simulação:** `100` (Para logs rápidos).
* **FPGA 50MHz:** `50000`.
* **FPGA 100MHz:** `100000`.

---

**Desenvolvido por Fábio Giacomel**
*Projeto Tesser Silicon - Jan/2026*
