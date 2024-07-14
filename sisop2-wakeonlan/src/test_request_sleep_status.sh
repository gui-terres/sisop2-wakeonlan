#!/bin/bash

unset GTK_PATH

# Iniciar todos os contêineres
docker-compose up -d

# Abrir um novo terminal para seguir a saída do contêiner manager
gnome-terminal -- bash -c "docker-compose logs -f manager; exec bash"

# Aguardar um tempo antes de desligar o contêiner (por exemplo, 10 segundos)
sleep 5

# Desligar o contêiner participant1
docker-compose stop participant1

# Aguardar um tempo antes de religar o contêiner (por exemplo, 5 segundos)
sleep 10

# Religar o contêiner participant1
docker-compose start participant1

sleep 10
