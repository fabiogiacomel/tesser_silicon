#!/bin/bash
set -e # Encerra o script se qualquer comando falhar

echo "========================================"
echo "🛡️  TESSER SILICON QUALITY GATE"
echo "========================================"

# Passo 1: Limpeza
echo "[1/3] Limpando artefatos anteriores..."
if [ -f Makefile ]; then
    make clean
else
    echo "⚠️  Makefile não encontrado. Pulando clean."
fi
rm -f tesser_tower

# Passo 2: Compilação Estrita
echo "[2/3] Compilando Emulador (Stack Machine)..."
# Se não houver Makefile, criamos um temporário ou assumimos comando direto
if [ ! -f Makefile ]; then
    echo "⚠️  Gerando Makefile de emergência..."
    echo "CC = gcc" > Makefile
    echo "CFLAGS = -Wall -Werror -I./src" >> Makefile
    # Ajustado para incluir apenas arquivos existentes
    echo "SRC = src/main.c src/tesser_cpu.c src/tesser_bus.c src/tesser_telemetry.c" >> Makefile
    echo "all:" >> Makefile
    echo -e "\t\$(CC) \$(CFLAGS) -o tesser_tower \$(SRC)" >> Makefile
    echo "clean:" >> Makefile
    echo -e "\trm -f tesser_tower" >> Makefile
fi

make all

if [ ! -f tesser_tower ]; then
    echo "❌ [ERRO] Binário 'tesser_tower' não foi gerado."
    exit 1
fi

# Passo 3: Teste de Fumaça (Smoke Test)
echo "[3/3] Executando Smoke Test (--test)..."
./tesser_tower --test

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ [SUCESSO] Build Estável. O sistema está íntegro."
else
    echo "❌ [ERRO] O emulador falhou durante o teste."
    exit 1
fi
