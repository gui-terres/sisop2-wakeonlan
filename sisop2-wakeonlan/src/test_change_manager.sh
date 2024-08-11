#!/bin/bash

unset GTK_PATH

# Iniciar os contêineres dos participantes e o manager
docker-compose up -d participant1 participant2 manager2

# Monitorar os logs dos contêineres dos participantes
gnome-terminal -- bash -c "docker-compose logs -f participant1 participant2; exec bash"

# Aguardar até que os contêineres dos participantes estejam prontos (por exemplo, 10 segundos)
sleep 3

# Desligar o contêiner do manager
docker-compose stop manager2

# Aguardar um tempo antes de iniciar o contêiner do manager2 (por exemplo, 10 segundos)
sleep 3

# Iniciar o contêiner do manager2
docker-compose up -d manager

# Aguardar um tempo para garantir que o manager2 tenha tempo suficiente para iniciar (por exemplo, 10 segundos)
sleep 3

# Desligar o contêiner do manager2
docker-compose stop manager

# Aguardar um tempo antes de reiniciar o contêiner do manager (por exemplo, 10 segundos)
sleep 3

# Reiniciar o contêiner do manager
docker-compose up -d manager2

echo "Contêineres gerenciados com sucesso."
