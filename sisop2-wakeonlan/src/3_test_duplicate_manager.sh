#!/bin/bash

unset GTK_PATH

# Função para iniciar os contêineres
start_service() {
    docker-compose up -d "$1"
}

# Função para parar os contêineres
suspend_service() {
    docker-compose pause "$1"
}

wake_service() {
    docker-compose unpause "$1"
}

# Função para monitorar logs dos participantes
monitor_logs() {
    gnome-terminal -- bash -c "docker-compose logs -f participant1; exec bash"
    gnome-terminal -- bash -c "docker-compose logs -f participant2; exec bash"
}

# Iniciar o participant1 (Estação A)
start_service participant1
start_service participant2

monitor_logs

sleep 25
docker-compose down

echo "Contêineres gerenciados e parados com sucesso."
