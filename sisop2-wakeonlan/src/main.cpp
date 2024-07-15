#include <iostream>
#include <cstdlib>
#include <string.h>
#include <thread>
#include <vector>
#include <iomanip>
#include <mutex>
#include "subsystems/discovery/discovery.hpp"
#include "subsystems/interface/interface.hpp"


using namespace std;
std::mutex cout_mutex;
std::string current_input;
char input[100];
int n = 0;

void runSendSocket(Server &server)
{
    server.sendSocket(BROADCAST_ADDR);
}

void requestParticipantsSleepStatus(Server &server)
{
    while (true) {
        // this_thread::sleep_for(chrono::seconds(5));
        RequestData req;
        req.request = Request::SLEEP_STATUS;

        for (const auto &client : discoveredClients) {
            Status status;
            if (server.requestSleepStatus(client.ipAddress, req, status) == 0) {
                cout << "Status do cliente " << client.hostname << ": " << (status == AWAKEN ? "AWAKEN" : "ASLEEP") << endl;
            }
        }
    }
}

// GERENCIAMENTO - sem WakeOnLan
void display(Server &server)
{
    while (true) {
        //cout << "tentando imprimir clientes" << endl;
        std::lock_guard<std::mutex> lock(cout_mutex);
        drawInterface();
        cout << "Para sair, digite EXIT" << endl;
        cout << "Para acordar um computador, digite WAKE + endereço ip" << endl;

        if (!discoveredClients.empty()) {

            std::cout << std::endl;
            std::cout << " _________________________________________________________" << std::endl;
            std::cout << "|              |                   |             |        |" << std::endl;
            std::cout << "|   Hostname   |   Endereço MAC    | Endereço IP | Status |" << std::endl;
            std::cout << "|______________|___________________|_____________|________|" << std::endl;
         
            for (const auto &client : discoveredClients) {
                std::cout << "|              |                   |             |        |" << std::endl;
                std::cout << "| " << std::setw(12) << client.hostname
                        << " | " << std::setw(17) << client.macAddress
                        << " | " << std::setw(11) << client.ipAddress
                        << " | " << std::setw(6) << client.status
                        << " |" << std::endl;
                std::cout << "|______________|___________________|_____________|________|" << std::endl;
            }
        } else {
          cout << endl << "Sem clientes no momento!" << endl << endl << endl;
        }
        //cout.unlock();
       // std::cout << "> " << string(input) << endl;
        
        this_thread::sleep_for(chrono::seconds(5));
    }
}

void searchForManager(Client &client, int argc, Status status)
{
    client.sendSocket(argc, status);
}

void waitForRequestsClient(Client &client, Status status)
{
    client.waitForRequests(status);
}

void sendExitRequest(Client &client, char *ip)
{   
    this_thread::sleep_for(chrono::seconds(10));    
    cout << ip << endl;

    if (!strcmp(ip, "172.18.0.12")){ //TODO
        client.sendExitRequest(client.managerInfo.ipAddress);      
    } else {
        cout << "eu estou vivo!!!!!1 (" << ip << ")" << endl;
    }
}

void waitForParticipantDataRequests(Client &client)
{
    client.waitForParticipantDataRequests();
}

void waitForRequestsServer(Server &server)
{
    server.waitForRequests(server);
}

void requestParticipantData(Server &server)
{   
    for (const auto &client : discoveredClients) {
        server.requestParticipantData(client.ipAddress);
    }
}

void sendWoLPacket(Server &server)
{
    // this_thread::sleep_for(chrono::seconds(3));
    while (true) {
        for (DiscoveredData &client : discoveredClients) {
            if (!strcmp(client.hostname, "s-67-101-15")) {  // Argument of command WAKEUP hostname
                cout << "tchaau" << endl;
                server.sendWoLPacket(client);
            }
        }
    }
}

void clear_line() {
    std::cout << "\33[2K\r";
}

void read_input();
void manipulateInput(char input[100]){
    std::string word(input);
    bool startsWithWake = (word.length() >= 4 && word.substr(0, 4) == "WAKE");
    if (word == "EXIT") {
        std::cout << "sairrr" << std::endl;
        std::exit(EXIT_SUCCESS); 
        } else if (startsWithWake) {
            std::cout << "WAKE" << std::endl;
            word.erase(0, 5);
            cout << "IP: " << word << endl; //IP TA AQUI!!!!!!!! -------------------------------------------
            } else {
                std::cout << "Comando inválido!" << std::endl;
            }
    read_input();
}

void read_input() {
    setTermNoBufferedInput(); // Configura terminal para entrada sem buffer
    n=0;
    char ch;
    for (int i = 0; i < 100; ++i) {
        input[i] = '\0';
    }
    while (true) {
        if (read(STDIN_FILENO, &ch, 1) == 1) { // Lê um caractere do terminal
            if (ch == '\b') { // Verifica se a tecla pressionada é o backspace
                if (n > 0) {
                    n--;
                    input[n] = '\0';
                }
            } else if (ch == '\n') { // Verifica se a tecla pressionada é o Enter (nova linha)
                manipulateInput(input);
                break; // Sai do loop ao pressionar Enter
            } else {
                input[n++] = ch; // Armazena o caractere lido e incrementa n
                input[n] = '\0'; // Garante que o array seja terminado corretamente
            }
            std::cout << "\033[1F"; // Move o cursor para cima em uma linha
            std::cout << "\033[2K"; // Limpa a linha atual
            cout << string(input) << endl;
        }
    }
}



void runManagerMode() {
    cout << "Manager mode" << endl;
    Server server;

    thread t1(runSendSocket, ref(server));
    // thread t2(requestParticipantsSleepStatus, ref(server));
    thread t3(display, ref(server));
    // thread t4(sendWoLPacket, ref(server));
    thread t5(waitForRequestsServer, ref(server));
    // thread t6(requestParticipantData, ref(server));
    thread t6(read_input);

    t1.join();
    // t2.join();
    t3.join();
    //t4.join();
    t5.join();
    t6.join();
}

void runClientMode(int argc, char *ip) {
    drawInterface();
    cout << "Client mode" << endl;
    Client client;
    Status status = Status::AWAKEN;

    thread t3(searchForManager, ref(client), argc, status);
    thread t4(waitForRequestsClient, ref(client), status);
    thread t5(sendExitRequest, ref(client), ip);
    thread t6(waitForParticipantDataRequests, ref(client));

    t3.join();
    t4.join();
    t5.join();
    t6.join();
}

int main(int argc, char **argv)
{
    if (argc > 3) {
        cout << "Invalid initialization!" << endl;
        return 1;
    }

    if (argc < 3) {
        if (!strcmp(argv[1], "manager")) {
            runManagerMode();
            return 0;
        } else {
            cout << "ERROR: Invalid argument initialization!" << endl;
            cout << argv[1] << " isn't a valid mode." << endl;
            return 1;
        }
    } else {
        runClientMode(argc, argv[1]);
        return 0;
    }

    
}