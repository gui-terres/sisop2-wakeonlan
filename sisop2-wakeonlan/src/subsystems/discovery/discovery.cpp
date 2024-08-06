#include "../../stations/stations.hpp"
#include "./discovery.hpp"

static void Discovery::discoverParticipants(Server &manager) {
    manager.sendSocket(BROADCAST_ADDR);
}

static void Discovery::searchForManager(Client &client, int argc) {
    while (!strcmp(client.managerInfo.ipAddress, PLACEHOLDER)) {
        client.sendSocket(argc);
    }
}