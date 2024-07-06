# Dockerfile
FROM ubuntu:latest

# Evitar prompts interativos
ENV DEBIAN_FRONTEND=noninteractive

# Instalação de dependências
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    iputils-ping \
    net-tools \
    vim \
    ntp \
    systemd \ 
    systemd-sysv

# Criação do diretório de trabalho
WORKDIR /app

# Copie o Makefile e os arquivos de código fonte mantendo a estrutura
COPY makefile /app/Makefile
COPY main.cpp /app/
COPY subsystems/discovery/client.cpp /app/subsystems/discovery/
COPY subsystems/discovery/server.cpp /app/subsystems/discovery/
COPY subsystems/discovery/discovery.hpp /app/subsystems/discovery/
# Se houver o arquivo interface.cpp, descomente a linha abaixo:
COPY subsystems/interface/interface.cpp /app/subsystems/interface/
COPY subsystems/interface/interface.hpp /app/subsystems/interface/

# Configure a sincronização de tempo (substituto para systemd-timesyncd)
RUN service ntp start

# Compile o projeto usando o Makefile
RUN make

# Verifique se o arquivo foi compilado corretamente
RUN ls -l /app

# Comando padrão
CMD ["/lib/systemd/systemd"]
