FROM debian:bullseye-slim

# Evitar prompts interativos durante a instalação
ENV DEBIAN_FRONTEND=noninteractive

# Atualizar repositórios e instalar dependências básicas de compilação
RUN apt-get update && apt-get install -y \
    build-essential \
    curl \
    git \
    gnupg \
    && rm -rf /var/lib/apt/lists/*

# Instalar Node.js 18.x (LTS)
RUN curl -fsSL https://deb.nodesource.com/setup_18.x | bash - \
    && apt-get install -y nodejs \
    && rm -rf /var/lib/apt/lists/*

# Definir o diretório de trabalho
WORKDIR /app

# Comando padrão caso não seja sobrescrito (útil para debug isolado)
CMD ["/bin/bash"]
