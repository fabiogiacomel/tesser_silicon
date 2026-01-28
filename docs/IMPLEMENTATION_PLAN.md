# Plano de Implementação: Tesser Silicon v1.0

Este documento define o roteiro de implementação para o processador soft-core Tesser, uma máquina de pilha de 16-bits.

## Fase 1: Fundação e Estrutura (Concluído)
- [x] Definição da Arquitetura de Pastas (`src`, `tb`, `sim`, `docs`).
- [x] Especificação da ISA (Tesser v1.0) no README.
- [x] Criação deste Plano de Implementação.

## Fase 2: Design dos Módulos Principais (RTL)
Nesta fase, implementaremos os blocos construtivos fundamentais do processador em `src/`.

### 2.1. Stack (Pilha)
*   **Arquivo**: `src/tesser_stack.v`
*   **Função**: Armazenar operandos e resultados intermediários.
*   **Specs**: LIFO (Last-In, First-Out), 16-bit wide, profundidade configurável (ex: 256 words). Sinais de `push`, `pop`, `data_in`, `data_out`, `full`, `empty`.

### 2.2. Program Counter (PC)
*   **Arquivo**: `src/tesser_pc.v`
*   **Função**: Manter o endereço da próxima instrução.
*   **Specs**: Registrador de 16 bits. Suporte a incremento (próxima instrução) e carga paralela (para `JMP` e `JMP_POS`).

### 2.3. ALU (Arithmetic Logic Unit)
*   **Arquivo**: `src/tesser_alu.v`
*   **Função**: Executar operações aritméticas.
*   **Specs**: Operação inicial principal: `SUB`. Pass-through para outras operações se necessário.

### 2.4. Control Unit (FSM & Decoder)
*   **Arquivo**: `src/tesser_control.v`
*   **Função**: O cérebro do processador. Busca instruções, decodifica opcodes e gera sinais de controle.
*   **Máquina de Estados**:
    1.  **FETCH**: Busca instrução na memória de programa.
    2.  **DECODE**: Identifica o Opcode.
    3.  **EXECUTE**: Executa a operação (manipula pilha, atualiza PC, interage com MUI).

## Fase 3: Integração (Top Level)
### 3.1. CPU Core
*   **Arquivo**: `src/tesser_cpu.v`
*   **Função**: Instanciar e conectar Stack, PC, ALU e Control Unit.
*   **Interface**:
    *   Entradas: `clk`, `rst_n`.
    *   Memória: `i_mem_data` (leitura instr), `o_mem_addr` (endereço instr).
    *   MUI: `i_mui_data`, `o_mui_data`, `o_mui_id`, `o_mui_wr_en`, `o_mui_rd_en`.

## Fase 4: Simulação e Verificação
Para cada módulo e para o processador completo, criaremos testbenches em `tb/`.

### 4.1. Unit Tests
*   `tb/tb_stack.v`: Verificar push, pop, bordas (cheio/vazio).
*   `tb/tb_alu.v`: Verificar operações aritméticas.

### 4.2. Integration Test (Hello World)
*   **Arquivo**: `tb/tb_tesser_core.v`
*   **Cenário**: Carregar um "programa" simulado na memória de instruções que exercite todos os Opcodes:
    1.  PUSH valores.
    2.  Executar SUB.
    3.  Testar JMP e JMP_POS.
    4.  Testar MUI_SET e MUI_GET.

## Próximos Passos Imediatos
1.  Implementar `tesser_stack.v`.
2.  Implementar `tesser_alu.v`.
3.  Implementar `tesser_cpu.v` (Top Level esqueleto).
