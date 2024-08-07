#include "./discovery.hpp"
#include <thread>
#include <chrono>

void Discovery::discoverParticipants(Server &manager) {
    manager.collectParticipants(BROADCAST_ADDR);
}

void Discovery::searchForManager(Client &client, int argc) {
    while (true){
        if (!strcmp(client.managerInfo.ipAddress, PLACEHOLDER)) {
            std::cout << "clientetetete oiii server??????" << std::endl;
            client.enterWakeOnLan(argc);
        } else{
            std::cout << "já tenho manager!!!!" << std::endl;
            std::cout << client.managerInfo.ipAddress << std::endl;
        }
    }
}