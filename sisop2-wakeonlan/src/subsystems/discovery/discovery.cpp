#include "./discovery.hpp"
#include <thread>
#include <chrono>

using namespace std;

void Discovery::discoverParticipants(Server &manager) {
    cout << "oiii" << endl;
    manager.collectParticipants(BROADCAST_ADDR);
}

void Discovery::searchForManager(Client &client, int argc) {
    while (!stopThreads.load()){
        client.getManagerData();
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void Discovery::enterWakeOnLan(Client &client, int argc){
    while(!stopThreads.load()){
        client.enterWakeOnLan(argc);
        this_thread::sleep_for(chrono::seconds(1));
    }
}