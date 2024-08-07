#include "./discovery.hpp"
#include <thread>
#include <chrono>

void Discovery::discoverParticipants(Server &manager) {
    manager.collectParticipants(BROADCAST_ADDR);
}

void Discovery::searchForManager(Client &client, int argc) {
    while (true){
        //checar timeout aqui
        if (!strcmp(client.managerInfo.ipAddress, PLACEHOLDER) || !strcmp(client.managerInfo.ipAddress, "")) {
            // std::cout << "clientetetete oiii server??????" << std::endl;
            client.getManagerData();
        } else{
            // std::cout << "já tenho manager!!!!" << std::endl;
            // std::cout << client.managerInfo.ipAddress << std::endl;
        }
    }
}

void Discovery::enterWakeOnLan(Client &client, int argc){
    while(true){
        client.enterWakeOnLan(argc);
    }
}