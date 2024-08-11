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

    t1.join();
    t2.join();
    t3.join();
    t5.join();
    t6.join();
    t8.join();    
}

// Função principal para executar o modo de cliente
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
    thread t13(Management::checkAndElectClient, ref(client));
    thread t10(Discovery::enterWakeOnLan, ref(client), argc);
    thread t7(read_input, ref(client), ref(server));
    thread t8(isCTRLc);
    thread t9(isCTRLcT, ref(client));

    // Thread para verificar a atualização do tipo
    thread upgradeThread(upgradeClientToManager, ref(client));

    while (!stopThreads.load()) {
        this_thread::sleep_for(chrono::seconds(1)); // Ajuste o intervalo conforme necessário
    }

    // Espera todas as threads terminarem antes de iniciar o modo de gerente
    if (type == Type::MANAGER) {
        cout << "Transição para o modo de gerente..." << endl;
        t3.join();
        cout << "oiiii" << endl;
        t4.join();
        cout << "oiiii" << endl;
        t7.join();
        cout << "oiiii" << endl;
        t8.join();
        cout << "oiiii" << endl;
        t9.join();
        cout << "oiiii" << endl;
        t10.join();
        cout << "oiiii" << endl;
        t12.join();
        cout << "oiiii" << endl;
        t13.join();
        cout << "oiiii" << endl;
        // Inicia o modo de gerente
        runManagerMode(isDocker);
    }

    t3.join();
    cout << "oiiii" << endl;
    t4.join();
    cout << "oiiii" << endl;
    t7.join();
    cout << "oiiii" << endl;
    t8.join();
    cout << "oiiii" << endl;
    t9.join();
    cout << "oiiii" << endl;
    t10.join();
    cout << "oiiii" << endl;
    t12.join();
    cout << "oiiii" << endl;
    t13.join();
    cout << "oiiii" << endl;    

    // Espera a thread de upgrade terminar
    upgradeThread.join();
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
