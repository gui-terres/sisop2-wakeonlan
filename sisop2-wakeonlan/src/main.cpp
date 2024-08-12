#include <iostream>
#include <cstdlib>
#include <string.h>
#include <thread>
#include <vector>
#include <iomanip>
#include <mutex>
#include <csignal> // Para signal e SIGINT
#include <chrono>  // Para std::this_thread::sleep_for

#include "./stations/stations.hpp"
#include "./subsystems/discovery/discovery.hpp"
#include "./subsystems/interface/interface.hpp"
#include "./subsystems/monitoring/monitoring.hpp"
#include "./subsystems/management/management.hpp"

// Instanciação de subserviços
Monitoring monitoring;
Management management;
Type type;

using namespace std;

void sendExitRequest(Client &client)
{   
    client.sendExitRequest(client.managerInfo.ipAddress);      
}
void sendPeriodicMessage(Client &client) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(4));
        client.sendMessage("oiii", client.managerInfo.ipAddress);
        
    }
}
void read_input(Client &client, Server &server);

void runManagerMode(bool isDocker = false) {
    cout << "Manager mode" << (isDocker ? " [Docker]" : "") << endl;
    Server server;
    Client client;
    type = Type::MANAGER;

    thread t1(Discovery::discoverParticipants, ref(server));
    thread t2(&Server::sendManagerInfo, &server);
    thread t3(Management::displayServer, ref(server));
    thread t5(&Server::waitForRequests, &server);
    thread t6(read_input, ref(client), ref(server));
    thread t7(&Server::receiveMessages, &server);

    t1.join();
    t2.join();
    t3.join();
    t5.join();
    t6.join();
    t7.join();
}

void runClientMode(int argc, bool isDocker = false) {
    !isDocker ? drawInterface() : void();
    cout << "Client mode" << (isDocker ? " [Docker]" : "") << endl;
    Server server;
    Client client;
    type = Type::PARTICIPANT;

    client.enterWakeOnLan(argc);

    thread t3(Discovery::searchForManager, ref(client), argc);
    thread t4(&Client::waitForSleepRequests, &client);
    thread t12(Management::displayClient, ref(client));
    thread t10(Discovery::enterWakeOnLan, ref(client), argc);
    thread t7(read_input, ref(client), ref(server));
    thread t8(isCTRLc);
    thread t9(isCTRLcT, ref(client));

    thread t13(sendPeriodicMessage, ref(client));

    t3.join();
    t4.join();
    t7.join();
    t8.join();
    t9.join();
    t10.join();
    t12.join();
    t13.join();
}

int main(int argc, char **argv)
{
    if (argc > 3) {
        cout << "Invalid initialization!" << endl;
        return 1;
    }
    
    Station station;

    bool isDocker = (argc == 3 && strcmp(argv[2], "docker") == 0);

    if (argc > 1) {
        if (!strcmp(argv[1], "manager")) {
            runManagerMode(isDocker);
            return 0;
        } else if (!isDocker) {
            cout << "ERROR: Invalid argument initialization!" << endl;
            cout << argv[1] << " isn't a valid mode." << endl;
            return 1;
        }else{
            runClientMode(argc, isDocker);
            return 0;           
        }
    } else {
        runClientMode(argc, isDocker);
        return 0;
    }   
}
