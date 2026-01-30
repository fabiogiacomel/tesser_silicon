# Base Image: Python 3.10 (Leve e robusto)
FROM python:3.10-slim

# Instalar GCC e Make para compilar o emulador em C
RUN apt-get update && apt-get install -y \
    gcc \
    make \
    && rm -rf /var/lib/apt/lists/*

# Definir diretório de trabalho
WORKDIR /app

# Instalar biblioteca para WebSocket
RUN pip install websockets

# Copiar todo o código do projeto para dentro do container
COPY . .

# Expor a porta 3000 (HTTP) e 8765 (WebSocket)
EXPOSE 3000
EXPOSE 8765

# O comando de entrada é o nosso Orchestrator em Python
CMD ["python", "server.py"]
