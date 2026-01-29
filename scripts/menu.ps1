<#
.SYNOPSIS
    Tesser Silicon Development Menu v2.0
    Gerenciamento unificado: FPGA (Verilog) e Emulação (C/Docker).
#>

$VERILOG_PATH = "C:\iverilog\bin\iverilog.exe" # Ajuste conforme seu caminho de instalação local
$VVP_PATH = "C:\iverilog\bin\vvp.exe"
$GTKWAVE_PATH = "C:\iverilog\gtkwave\bin\gtkwave.exe"

$ROOT_DIR = Resolve-Path "$PSScriptRoot\.."
$SRC_DIR = "$ROOT_DIR\src"
$SIM_DIR = "$ROOT_DIR\sim"

function Show-Header {
    Clear-Host
    Write-Host "==============================" -ForegroundColor Cyan
    Write-Host "   TESSER SILICON DEV KIT v2  " -ForegroundColor Yellow
    Write-Host "==============================" -ForegroundColor Cyan
    Write-Host ""
}

# --- DOCKER OPERATIONS ---
function Start-Docker {
    Write-Host "[Docker] Iniciando Tesser Lab..." -ForegroundColor Cyan
    Set-Location $ROOT_DIR
    docker-compose up -d --build
    if ($LASTEXITCODE -eq 0) { Write-Host "-> Tesser Lab Online." -ForegroundColor Green }
    else { Write-Host "-> Erro ao iniciar Docker." -ForegroundColor Red }
}

function Enter-Docker {
    Write-Host "[Docker] Entrando no Shell..." -ForegroundColor Cyan
    Set-Location $ROOT_DIR
    docker-compose exec tesser_lab bash
}

function Stop-Docker {
    Write-Host "[Docker] Desligando Tesser Lab..." -ForegroundColor Yellow
    Set-Location $ROOT_DIR
    docker-compose down
}

function Build-Run-C-Emulator-Docker {
    Write-Host "[Docker] Compilando e Rodando Emulador C..." -ForegroundColor Cyan
    Set-Location $ROOT_DIR
    # Compila e roda dentro do container (Modo Texto Simples)
    # Usa main.c que tem o loop infinito, então o usuário precisará dar Ctrl+C
    docker-compose exec tesser_lab bash -c "gcc src/main.c src/tesser_cpu.c src/tesser_bus.c src/tesser_peripherals.c src/tesser_memory.c src/tesser_telemetry.c -o tesser_vm && ./tesser_vm"
}

function Run-Watchtower-Docker {
    Write-Host "[Docker] Iniciando Watchtower (Servidor + Bridge)..." -ForegroundColor Cyan
    Write-Host "Acesse http://localhost:3000/vd/index.html (se servido) ou abra o arquivo localmente." -ForegroundColor Gray
    
    Set-Location $ROOT_DIR
    
    # 1. Compila o binário 'tesser_tower' específico para a bridge
    # 2. Roda a bridge que spawna o binário
    docker-compose exec tesser_lab bash -c "gcc src/main.c src/tesser_cpu.c src/tesser_bus.c src/tesser_peripherals.c src/tesser_memory.c src/tesser_telemetry.c -o tesser_tower && node bridge.js"
}

# --- VERILOG/FPGA OPERATIONS (Local) ---
function Run-Assembler {
    param($InputFile)
    Write-Host "[FPGA] Executando Assembler (TASM)..." -ForegroundColor Magenta
    python "$ROOT_DIR\tasm.py" "$ROOT_DIR\$InputFile"
    if ($LASTEXITCODE -eq 0) { Write-Host "-> firmware.hex gerado." -ForegroundColor Green }
}

function Run-Simulation {
    Write-Host "[FPGA] Compilando Verilog Localmente..." -ForegroundColor Magenta
    if (-not (Test-Path $SIM_DIR)) { New-Item -ItemType Directory -Path $SIM_DIR | Out-Null }
    & $VERILOG_PATH -o "$SIM_DIR\tesser.out" -s tb_tesser "$SRC_DIR\tesser_cpu.v" "$ROOT_DIR\tb\tb_tesser.v"
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "[FPGA] Executando Simulação..." -ForegroundColor Magenta
        & $VVP_PATH "$SIM_DIR\tesser.out"
    }
    else {
        Write-Host "-> Falha no Iverilog. Verifique se está instalado." -ForegroundColor Red
    }
}

function Open-GTKWave {
    Write-Host "[FPGA] Abrindo GTKWave..." -ForegroundColor Magenta
    if (Test-Path "$SIM_DIR\tesser.vcd") {
        Start-Process $GTKWAVE_PATH -ArgumentList "$SIM_DIR\tesser.vcd"
    }
    else {
        Write-Host "-> VCD não encontrado. Rode a simulação primeiro." -ForegroundColor Red
    }
}

# Loop Principal
do {
    Show-Header
    Write-Host "--- AMBIENTE DOCKER (Tesser Lab) ---" -ForegroundColor Green
    Write-Host "1. Iniciar Lab (Up -d)"
    Write-Host "2. Entrar no Shell (Exec bash)"
    Write-Host "3. Compilar e Rodar Emulador C (Terminal Mode)"
    Write-Host "4. Rodar Watchtower (Server + Bridge JSON)"
    Write-Host "5. Desligar Lab (Down)"
    
    Write-Host "`n--- HARDWARE FLOW (Local Verilog) ---" -ForegroundColor Magenta
    Write-Host "6. Montar 'pisca.tasm' (Assembler)"
    Write-Host "7. Simular RTL (Iverilog)"
    Write-Host "8. Ver Ondas (GTKWave)"
    
    Write-Host "`nQ. Sair"
    Write-Host ""
    $choice = Read-Host "Escolha"

    switch ($choice) {
        '1' { Start-Docker; Pause }
        '2' { Enter-Docker }
        '3' { Build-Run-C-Emulator-Docker; Pause }
        '4' { Run-Watchtower-Docker; Pause }
        '5' { Stop-Docker; Pause }
        '6' { Run-Assembler "pisca.tasm"; Pause }
        '7' { Run-Simulation; Pause }
        '8' { Open-GTKWave }
        'Q' { break }
        'q' { break }
    }
} while ($true)
