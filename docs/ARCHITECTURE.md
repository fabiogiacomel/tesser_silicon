# Arquitetura do Tesser Silicon

Esta página descreve a arquitetura interna do processador `tesser_cpu` baseada na implementação RTL Verilog.

## Diagrama de Blocos (Mermaid)

O diagrama abaixo ilustra o fluxo de dados e controle entre a Memória de Programa (ROM), a Unidade de Controle (FSM), a Pilha (Stack) e a Interface de Periféricos (MUI).

```mermaid
graph TD
    subgraph "Memory & Fetch"
        PC[Program Counter (16-bit)] -->|Addr| ROM[Program ROM (256 bytes)]
        ROM -->|Data| IR[Opcode / Args]
    end

    subgraph "Control Unit (FSM)"
        IR --> Decoder
        Decoder -->|State Transition| FSM{State Machine}
        FSM -->|Control Signals| ALU
        FSM -->|Control Signals| StackControl
        FSM -->|Control Signals| MUI_Ctrl
        
        stateFetch[FETCH] --> stateDecode[DECODE]
        stateDecode --> stateExecute[EXECUTE]
        stateExecute --> stateFetch
    end

    subgraph "Data Path"
        StackControl --> SP[Stack Pointer (8-bit)]
        SP --> StackMem[Stack Memory (16x16-bit)]
        StackMem <-->|Push/Pop| ALU[ALU / Execution Logic]
    end

    subgraph "MUI Interface (I/O)"
        ALU -->|Value| MUI_VAL[mui_val (16-bit)]
        Decoder -->|ID| MUI_ID[mui_id (8-bit)]
        MUI_Ctrl -->|Write Strobe| MUI_WE[mui_we]
        MUI_Ctrl -->|Read Strobe| MUI_RE[mui_re]
        MUI_RX[mui_rx_val (Input)] -->|Read| ALU
    end

    style ROM fill:#f9f,stroke:#333
    style FSM fill:#ff9,stroke:#333
    style StackMem fill:#9ff,stroke:#333
    style MUI_VAL fill:#f90,stroke:#333
```

## Descrição dos Módulos

### 1. Fetch & Decode
*   **PC (Program Counter):** Aponta para a próxima instrução na ROM. Incrementado a cada byte lido ou atualizado por saltos (`JMP`).
*   **ROM:** Armazena o firmware (`firmware.hex`). O código é lido byte a byte.
*   **FSM:** Máquina de estados principal com 4 estados (`FETCH`, `DECODE_1`, `DECODE_2`, `EXECUTE`).

### 2. Stack Machine
*   **SP (Stack Pointer):** Aponta para o topo da pilha.
*   **Stack Memory:** LIFO de 16 posições de 16-bits. Todas as operações aritméticas (`SUB`) e de I/O logicamente ocorrem usando o topo da pilha.

### 3. MUI (Menor Unidade Indivisível)
A interface com o mundo externo (sensores e atuadores).
*   **Saída:** `MUI_SET` pega valores da pilha e escreve no barramento.
*   **Entrada:** `MUI_GET` usa o barramento para ler dados externos e impurrá-los para a pilha.
