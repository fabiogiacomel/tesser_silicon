<#
.SYNOPSIS
    Tesser Silicon Development Menu v2.1
    Gerenciamento Unificado: FPGA (Verilog/Local) e Emulação (C/Docker).
#>

# Paths para ferramentas locais (Verilog)
$VERILOG_PATH = "C:\iverilog\bin\iverilog.exe"
$VVP_PATH = "C:\iverilog\bin\vvp.exe"
$GTKWAVE_PATH = "C:\iverilog\gtkwave\bin\gtkwave.exe"

$ROOT_DIR = Resolve-Path "$PSScriptRoot\.."
$SRC_DIR = "$ROOT_DIR\src"
$SIM_DIR = "$ROOT_DIR\sim"

function Show-Header {
    Clear-Host
    Write-Host "==============================" -ForegroundColor Cyan
    Write-Host "   TESSER SILICON DEV KIT v2.1" -ForegroundColor Yellow
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

function Verify-Build-In-Docker {
    Write-Host "[Docker] Rodando Protocolo QA (verify.sh)..." -ForegroundColor Cyan
    Set-Location $ROOT_DIR
    # Roda script de verificação dentro do container
    docker-compose exec tesser_lab bash -c "./verify.sh"
}

function Run-Watchtower-Docker {
    Write-Host "[Docker] Configurando Watchtower (Stack Machine)..." -ForegroundColor Cyan
    Set-Location $ROOT_DIR
    
    # QA Check Antes de Rodar
    # docker-compose exec tesser_lab bash -c "./verify.sh"
    # if ($LASTEXITCODE -ne 0) { 
    #     Write-Host "❌ Build Falhou no QA. Corrija antes de visualizar." -ForegroundColor Red
    #     return 
    # }

    Write-Host ">> Compilando e Iniciando Ponte..." -ForegroundColor Green
    Write-Host ">> ACESSE NO NAVEGADOR: http://localhost:3000" -ForegroundColor Yellow
    
    # 1. Limpa e Compila (Garante binário fresco)
    # 2. Inicia Bridge Servidor
    docker-compose exec tesser_lab bash -c "make clean && make && node bridge.js"
}

# --- VERILOG/FPGA OPERATIONS (Local) ---
function Run-Assembler {
    param($InputFile)
    Write-Host "[FPGA] Executando Assembler (TASM)..." -ForegroundColor Magenta
    Set-Location $ROOT_DIR
    python "tasm.py" "$InputFile"
    if ($LASTEXITCODE -eq 0) { Write-Host "-> firmware.hex gerado." -ForegroundColor Green }
}

function Run-Simulation {
    Write-Host "[FPGA] Compilando Verilog Localmente..." -ForegroundColor Magenta
    Set-Location $ROOT_DIR
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
    Set-Location $ROOT_DIR
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
    Write-Host "--- AMBIENTE DE EMULAÇÃO (Docker C) ---" -ForegroundColor Green
    Write-Host "1. Iniciar Lab (Up -d)"
    Write-Host "2. Entrar no Shell (Exec bash)"
    Write-Host "3. AUDITORIA DE QUALIDADE (Run verify.sh)"
    Write-Host "4. INICIAR WATCHTOWER (Visualizador Web)"
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
        '3' { Verify-Build-In-Docker; Pause }
        '4' { Run-Watchtower-Docker; Pause }
        '5' { Stop-Docker; Pause }
        '6' { Run-Assembler "pisca.tasm"; Pause }
        '7' { Run-Simulation; Pause }
        '8' { Open-GTKWave }
        'Q' { break }
        'q' { break }
    }
} while ($true)
