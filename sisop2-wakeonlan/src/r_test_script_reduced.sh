#!/bin/bash

unset GTK_PATH

# Função para iniciar os contêineres
start_service() {
    docker-compose up -d "$1"
}

# Função para parar os contêineres
stop_service() {
    docker-compose stop "$1"
}

# Função para monitorar logs dos participantes
monitor_logs() {
    gnome-terminal -- bash -c "docker-compose logs -f participant1; exec bash"
    gnome-terminal -- bash -c "docker-compose logs -f participant2; exec bash"
}

# Iniciar o participant1 (Estação A)
start_service participant1

# Monitorar os logs dos contêineres dos participantes
monitor_logs

# Aguardar até que participant1 esteja pronto para se identificar como líder
sleep 15

# Iniciar o participant2 (Estação B)
start_service participant2

# Aguardar até que participant2 tenha tempo para reconhecer o líder
sleep 10

# Colocar participant1 para dormir (parar o líder original)
stop_service participant1

# Aguardar até que uma nova eleição seja realizada
sleep 15

# Parar todos os contêineres
docker-compose down

echo "Contêineres gerenciados e parados com sucesso."
