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

using namespace std;

// Variáveis para controle de threads
// mutex typeMutex;
condition_variable cv;
atomic<bool> stopThreads(false); // Atômica para garantir visibilidade entre threads

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
                cout << "oiii" << endl;
                // Aguarda o término das threads do cliente
                // Threads do cliente devem verificar stopThreads
                return;
            }
        }
        this_thread::sleep_for(chrono::seconds(1)); // Verifica a cada segundo
    }
}

// Função principal para executar o modo de gerente
void runManagerMode(bool isDocker = false) {
    cout << "Manager mode" << (isDocker ? " [Docker]" : "") << endl;
    Server server;
    Client client;
    type = Type::MANAGER;

    stopThreads.store(false);


    thread t1(Discovery::discoverParticipants, ref(server));
    thread t2(&Server::sendManagerInfo, &server);
    thread t3(Management::displayServer, ref(server));
    thread t5(&Server::waitForRequests, &server);
    thread t6(read_input, ref(client), ref(server));
    thread t8(isCTRLc);
    //thread t7(&Server::receiveMessages, &server);
    thread t7(&Server::sendTable, &server);
    thread t9(&Server::sendCoordinatorMessage, &server);

    t1.join();
    t2.join();
    t3.join();
    t5.join();
    t6.join();
    t8.join();    
    t7.join();
}

// Função principal para executar o modo de cliente
void runClientMode(int argc, bool isDocker = false) {
    !isDocker ? drawInterface() : void();
    cout << "Client mode" << (isDocker ? " [Docker]" : "") << endl;
    Server server;
    Client client;
    type = Type::PARTICIPANT;
    id = getpid();
    
    thread t1(Discovery::searchForManager, ref(client), argc);
    thread t2(&Client::waitForSleepRequests, &client);
    thread t3(Management::displayClient, ref(client));
    thread t4(Management::checkAndElectClient, ref(client));
    thread t5(Discovery::enterWakeOnLan, ref(client), argc);
    thread t6(read_input, ref(client), ref(server));
    thread t7(isCTRLc);
    thread t8(isCTRLcT, ref(client));

    // Thread para escutar a porta COORDINATOR e iniciar eleição em caso de timeout
    thread t9(&Station::listenForCoordinator, &client);
    //thread t13(sendPeriodicMessage, ref(client));
    thread t13(&Client::askForTable, ref(client));

    // Thread para verificar a atualização do tipo
    thread t10(upgradeClientToManager, ref(client));

    while (!stopThreads.load()) {
        this_thread::sleep_for(chrono::seconds(1)); // Ajuste o intervalo conforme necessário
    }
    cout << "oiii" << endl;
    // Espera todas as threads terminarem antes de iniciar o modo de gerente
    if (type == Type::MANAGER) {
        cout << "Transição para o modo de gerente..." << endl;
        t1.join();
        t2.join();
        t3.join();
        t4.join();
        t5.join();
        t6.join();
        t7.join();
        t8.join();
        t9.join();
        t13.join();
        t10.join();         
        // Inicia o modo de gerente
        runManagerMode(isDocker);
    }

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
    t8.join();    
    t9.join();
    t13.join();
    t10.join();  // Espera as novas threads terminarem
}


// Função principal do programa
int main(int argc, char **argv) {
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
        } else {
            runClientMode(argc, isDocker);
            return 0;           
        }
    } else {
        runClientMode(argc, isDocker);
        return 0;
    }   
}
