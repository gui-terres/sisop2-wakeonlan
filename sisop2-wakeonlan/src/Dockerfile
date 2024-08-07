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
    systemd \ 
    systemd-sysv

# Criação do diretório de trabalho
WORKDIR /app

# Copie o Makefile e os arquivos de código fonte mantendo a estrutura
COPY makefile /app/Makefile
COPY main.cpp /app/
COPY stations/client.cpp /app/stations/
COPY stations/server.cpp /app/stations/
COPY stations/station.cpp /app/stations/
COPY stations/stations.hpp /app/stations/
COPY subsystems/discovery/discovery.cpp /app/subsystems/discovery/
COPY subsystems/discovery/discovery.hpp /app/subsystems/discovery/
# Se houver o arquivo interface.cpp, descomente a linha abaixo:
COPY subsystems/interface/interface.cpp /app/subsystems/interface/
COPY subsystems/interface/interface.hpp /app/subsystems/interface/
COPY subsystems/management/management.cpp /app/subsystems/management/
COPY subsystems/management/management.hpp /app/subsystems/management/
COPY subsystems/monitoring/monitoring.cpp /app/subsystems/monitoring/
COPY subsystems/monitoring/monitoring.hpp /app/subsystems/monitoring/

# Compile o projeto usando o Makefile
RUN make

# Verifique se o arquivo foi compilado corretamente
RUN ls -l /app

# Comando padrão
CMD ["bash"]
