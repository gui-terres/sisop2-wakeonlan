#include <iostream>
#include <cstdlib>
#include <string.h>
#include <thread>
#include <vector>
#include <iomanip>
#include <mutex>
#include <csignal> // Para signal e SIGINT

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

// void runSendSocket(Server &server)
// {
//     server.sendSocket(BROADCAST_ADDR);
// }

// void requestParticipantsSleepStatus(Server &server)
// {
//     RequestData req;
//     req.request = Request::SLEEP_STATUS;

//     for (DiscoveredData &client : discoveredClients) {
//         Status status;
//         if (server.requestSleepStatus(client.ipAddress, req, status) == 0) {
//             client.status = status;
//         }
//     }
// }

// GERENCIAMENTO - sem WakeOnLan
// void display(Server &server)
// {
//     while (true) {
        
//         requestParticipantsSleepStatus(server);
//         std::lock_guard<std::mutex> lock(cout_mutex);
//         drawInterface();
//         cout << "Para sair, digite EXIT" << endl;
//         cout << "Para acordar um computador, digite WAKE + endereço ip" << endl;

//         if (!discoveredClients.empty()) {
//             std::cout << std::endl;
//             std::cout << " ___________________________________________________________" << std::endl;
//             std::cout << "|              |                   |               |        |" << std::endl;
//             std::cout << "|   Hostname   |   Endereço MAC    |  Endereço IP  | Status |" << std::endl;
//             std::cout << "|______________|___________________|_______________|________|" << std::endl;
         
//             for (const auto &client : discoveredClients) {
//                 std::cout << "|              |                   |               |        |" << std::endl;
//                 std::cout << "| " << std::setw(12) << client.hostname
//                         << " | " << std::setw(17) << client.macAddress
//                         << " | " << std::setw(13) << client.ipAddress
//                         << " | " << std::setw(6) << client.status
//                         << " |" << std::endl;
//                 std::cout << "|______________|___________________|_______________|________|" << std::endl;
//             }
//         } else {
//           cout << endl << "Sem clientes no momento!" << endl << endl << endl;
//         }
//         cout << endl; 

//        this_thread::sleep_for(chrono::seconds(5));
//     }
// }



// void searchForManager(Client &client, int argc)
// {
//     while (!strcmp(client.managerInfo.ipAddress,"@@@@")){
//         client.sendSocket(argc);
//     }
// }

// void waitForRequestsClient(Client &client)
// {
//     client.waitForRequests();
// }

void sendExitRequest(Client &client)
{   
    client.sendExitRequest(client.managerInfo.ipAddress);      
}

// void waitForParticipantDataRequests(Client &client)
// {
//     client.waitForParticipantDataRequests();
// }

// void waitForRequestsServer(Server &server)
// {
//     server.waitForRequests(server);
// }

// void requestParticipantData(Server &server)
// {   
//     for (const auto &client : discoveredClients) {
//         server.requestParticipantData(client.ipAddress);
//     }
// }

// void clear_line() {
//     std::cout << "\33[2K\r";
// }

void read_input(Client &client, Server &server);

// void manipulateInput(char input[100], Client &client, Server &server){
//     std::string word(input);
//     bool startsWithWake = (word.length() >= 4 && word.substr(0, 4) == "WAKE");
//     if (word == "EXIT") {
//         std::cout << client.managerInfo.hostname << ": saindo do sistema..." << std::endl;
//         sendExitRequest(client);
//         restoreTermSettings();
//         std::exit(EXIT_SUCCESS); 
//     } else if (startsWithWake && type == 1) {
//         word.erase(0, 5);
//         sendWoLPacket(server, word);
//     } else {
//         std::cout << "Comando inválido!" << std::endl;
//     }
//     read_input(client, server);
// }

// void read_input(Client &client, Server &server) {
//     setTermNoBufferedInput(); // Configura terminal para entrada sem buffer
//     n=0;
//     char ch;
//     for (int i = 0; i < 100; ++i) {
//         input[i] = '\0';
//     }
//     while (true) {
//         if (read(STDIN_FILENO, &ch, 1) == 1) { 
//             if (ch == '\b') { 
//                 if (n > 0) {
//                     cout << "aaaa" << endl;
//                     n--;
//                     input[n] = '\0';
//                 }
//             } else if (ch == '\n') { 
//                 manipulateInput(input,client, server);
//                 break; 
//             } else {
//                 input[n++] = ch; 
//                 input[n] = '\0'; 
//             }
//             std::cout << "\033[1F"; // Move o cursor para cima em uma linha
//             std::cout << "\033[2K"; // Limpa a linha atual
//             cout << string(input) << endl;
//         }
//     }
// }

// void handleSigInt(int signum) {
//     ctrl = 1;
// }
// void isCTRLc(){
//     signal(SIGINT, handleSigInt);
// }
// void isCTRLcT(Client &client){
//     while (true)
//     {
//         if(ctrl){
//             std::cout << client.managerInfo.hostname << ": saindo do sistema..." << std::endl;
//         sendExitRequest(client);
//         restoreTermSettings();
//         std::exit(EXIT_SUCCESS); 
//         }
//     }
    
// }

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

    // thread t1(runSendSocket, ref(server));
    // thread t2(requestParticipantsSleepStatus, ref(server));
    // thread t3(display, ref(server));
    // thread t5(waitForRequestsServer, ref(server));
    // thread t6(read_input, ref(client), ref(server));

    t1.join();
    t2.join();
    t3.join();
    t5.join();
    t6.join();
    // t6.join();
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

    // thread t3(searchForManager, ref(client), argc);
    // thread t4(waitForRequestsClient, ref(client));
    // thread t7(read_input, ref(client), ref(server));
    // thread t8(isCTRLc);
    // thread t9(isCTRLcT, ref(client));

    t3.join();
    t4.join();
    t7.join();
    t8.join();
    t9.join();
    t10.join();
    t12.join();
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
