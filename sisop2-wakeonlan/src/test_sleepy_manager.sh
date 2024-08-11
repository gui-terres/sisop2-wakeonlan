#!/bin/bash

unset GTK_PATH

# Iniciar os contêineres dos participantes
docker-compose up -d participant1 participant2

gnome-terminal -- bash -c "docker-compose logs -f participant1 participant2; exec bash"

# Aguardar até que os contêineres dos participantes estejam prontos (por exemplo, 10 segundos)
sleep 5

# Iniciar o contêiner do manager
docker-compose up -d manager

# Abrir um novo terminal para seguir a saída dos contêineres participantes

# Aguardar um tempo antes de desligar o contêiner do manager (por exemplo, 10 segundos)
sleep 5

# Desligar o contêiner do manager
docker-compose stop manager

# Aguardar um tempo antes de religar o contêiner do manager (por exemplo, 5 segundos)
sleep 10

# Religar o contêiner do manager
docker-compose start manager

sleep 10

# Parar todos os contêineres
docker-compose down
