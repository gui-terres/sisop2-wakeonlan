#include "./discovery.hpp"
#include <thread>
#include <chrono>

using namespace std;

void Discovery::discoverParticipants(Server &manager) {
    manager.collectParticipants(BROADCAST_ADDR);
}

void Discovery::searchForManager(Client &client, int argc) {
    while (true){
        client.getManagerData();
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void Discovery::enterWakeOnLan(Client &client, int argc){
    while(true){
        client.enterWakeOnLan(argc);
    }
}