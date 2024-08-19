#include <iostream>
#include <cstdlib>
#include <string.h>
#include <thread>
#include <vector>
#include <iomanip>
#include <mutex>
#include <condition_variable>
#include <csignal> // Para signal e SIGINT
#include <atomic> // Para atomic
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
int id;
Server server;
Client client;
Station station;
StationData managerInfo = {
    hostname: PLACEHOLDER,
    ipAddress: PLACEHOLDER,
    macAddress: PLACEHOLDER,
    type: Type::MANAGER,
    status: Status::ASLEEP
};

using namespace std;

// Variáveis para controle de threads
// mutex typeMutex;
condition_variable cv;
atomic<bool> stopThreads(false); // Atômica para garantir visibilidade entre threads

void runClientMode(int argc, bool isDocker);

// Função para atualizar o tipo do cliente para gerente
void upgradeClientToManager(Client &client) {
    while (true) {
        {
            lock_guard<mutex> lock(mtx);
            if (type == Type::MANAGER) {
                cout << "Cliente se tornando gerente..." << endl;

                // Sinaliza para parar as outras threads
                stopThreads.store(true);
                cv.notify_all();

                // Aguarda o término das threads do cliente
                // Threads do cliente devem verificar stopThreads
                return;
            }
        }
        // this_thread::sleep_for(chrono::seconds(1)); // Verifica a cada segundo
    }
}

// Função para atualizar o tipo do gerente para cliente
void downgradeManagerToClient(Server &server) {
    while (true) {
        {
            lock_guard<mutex> lock(mtx);
            if (type == Type::PARTICIPANT) {
                cout << "Gerente se tornando cliente..." << endl;

                // Sinaliza para parar as outras threads
                stopThreads.store(true);
                cv.notify_all();

                // Aguarda o término das threads do gerente
                // Threads do gerente devem verificar stopThreads
                return;
            }
        }
        // this_thread::sleep_for(chrono::seconds(1)); // Verifica a cada segundo
    }
}

// Função principal para executar o modo de gerente
void runManagerMode(bool isDocker = false) {
    cout << "Manager mode" << (isDocker ? " [Docker]" : "") << endl;
    type = Type::MANAGER;

    StationData pcData;
    stopThreads.store(false);
    int sockfd = station.createSocket();  
    char buffer[BUFFER_SIZE];
    station.getHostname(buffer, BUFFER_SIZE, pcData);
    station.getIpAddress(pcData);
    station.getMacAddress(sockfd, pcData.macAddress, MAC_ADDRESS_SIZE);
    pcData.status = Status::AWAKEN;
    pcData.type = Type::MANAGER;
    close(sockfd);

    vector<thread> threads;

    if (discoveredClients.empty()){
        {
            std::lock_guard<std::mutex> lock(mtx);
            discoveredClients.push_back(pcData);            
        }
    }

    server.sendCoordinatorMessage();

    Management::updateClientTypeByIP(managerInfo.ipAddress, Type::SLEEPY_MANAGER);
    Management::updateClientTypeByIP(pcData.ipAddress, Type::MANAGER);

    managerInfo = pcData;

    threads.push_back(thread(&Server::listenOnPort, &server, PORT_DOWNGRADE)); 
    threads.push_back(thread(Discovery::discoverParticipants, ref(server)));
    threads.push_back(thread(&Server::sendManagerInfo, &server));
    threads.push_back(thread(&Station::waitForSleepRequests, &station));
    threads.push_back(thread(Management::displayServer, ref(server)));
    threads.push_back(thread(Monitoring::sendDowngradeToSleepyManagers, ref(server)));
    threads.push_back(thread(&Server::waitForRequests, &server));
    threads.push_back(thread(read_input, ref(client), ref(server)));
    threads.push_back(thread(isCTRLcT, ref(client)));
    threads.push_back(thread(&Server::sendTable, &server));
    threads.push_back(thread(downgradeManagerToClient, ref(server))); // Verifica quando deve se tornar cliente

    
    // Loop para verificar se as threads devem parar
    while (!stopThreads.load()) {
        this_thread::sleep_for(chrono::seconds(1)); // Ajuste o intervalo conforme necessário
    }

    // Verifica se houve transição para o modo de gerente
    if (type == Type::PARTICIPANT) {
        cout << "Transição para o modo de cliente..." << endl;
        
        // Aguarda todas as threads terminarem
        for (size_t i = 0; i < threads.size(); ++i) {
            cout << "Joining thread " << i << " with ID: " << threads[i].get_id() << endl;
            threads[i].join();
        }
        cout << "All threads have finished." << endl;
        // Inicia o modo de gerente
        runClientMode(0,isDocker);
    } else {
        // Aguarda todas as threads terminarem antes de finalizar o modo cliente
        for (size_t i = 0; i < threads.size(); ++i) {
            cout << "Joining thread " << i << " with ID: " << threads[i].get_id() << endl;
            if (threads[i].joinable()) {
                threads[i].join();
            }
        }
    }
}

void runClientMode(int argc, bool isDocker = false) {
    if (!isDocker) {
        drawInterface();
    }

    stopThreads.store(false);

    cout << "Client mode" << (isDocker ? " [Docker]" : "") << endl;
    type = Type::PARTICIPANT;

    // Inicializa as threads necessárias para o modo cliente
    vector<thread> clientThreads;
    clientThreads.push_back(thread(Discovery::searchForManager, ref(client), argc));
    clientThreads.push_back(thread(&Station::waitForSleepRequests, &station));
    clientThreads.push_back(thread(Management::displayClient, ref(client)));
    clientThreads.push_back(thread(Discovery::enterWakeOnLan, ref(client), argc));
    clientThreads.push_back(thread(read_input, ref(client), ref(server)));
    clientThreads.push_back(thread(isCTRLc));
    clientThreads.push_back(thread(isCTRLcT, ref(client)));
    clientThreads.push_back(thread(&Client::askForTable, ref(client)));
    clientThreads.push_back(thread(&Station::listenForElectionMessages, &station));

    // Thread para verificar a atualização do tipo de cliente para gerente
    clientThreads.push_back(thread(upgradeClientToManager, ref(client)));

    // Loop para verificar se as threads devem parar
    while (!stopThreads.load()) {
        this_thread::sleep_for(chrono::seconds(1)); // Ajuste o intervalo conforme necessário
    }

    // Verifica se houve transição para o modo de gerente
    if (type == Type::MANAGER) {
        cout << "Transição para o modo de gerente..." << endl;
        
        // Aguarda todas as threads terminarem
        for (size_t i = 0; i < clientThreads.size(); ++i) {
            cout << "Joining thread " << i << " with ID: " << clientThreads[i].get_id() << endl;
            clientThreads[i].join();
        }
        cout << "All threads have finished." << endl;
        // Inicia o modo de gerente
        runManagerMode(isDocker);
    } else {
        // Aguarda todas as threads terminarem antes de finalizar o modo cliente
        for (size_t i = 0; i < clientThreads.size(); ++i) {
            cout << "Joining thread " << i << " with ID: " << clientThreads[i].get_id() << endl;
            if (clientThreads[i].joinable()) {
                clientThreads[i].join();
            }
        }
    }
}



// Função principal do programa
int main(int argc, char **argv) {
    if (argc > 3) {
        cout << "Invalid initialization!" << endl;
        return 1;
    }

    bool isDocker = (argc == 3 && strcmp(argv[2], "docker") == 0);

    if (argc > 1) {
        if (!strcmp(argv[1], "manager")) {
            runManagerMode(isDocker);
            return 0;
        } else if (!isDocker) {
            cout << "ERROR: Invalid argument initialization!" << endl;
            cout << argv[1] << " isn't a valid mode." << endl;
            return 1;
        } else {
            runClientMode(argc, isDocker);
            return 0;           
        }
    } else {
        runClientMode(argc, isDocker);
        return 0;
    }   
}