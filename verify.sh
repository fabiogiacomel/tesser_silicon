#!/bin/bash
set -e

echo "--- 1. Cleaning ---"
make clean

echo "--- 2. Compiling (Strict Mode) ---"
make CFLAGS="-Wall -Werror -I./src" all

echo "--- 3. Smoke Test ---"
./tesser_tower --test

echo -e "\n\033[0;32m[SUCESSO] Build Estável.\033[0m"
