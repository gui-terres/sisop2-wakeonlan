#include "interface.hpp"
#include "../../stations/stations.hpp"

#include <iostream>
#include <cstdlib>
#include <string.h>
#include <thread>
#include <vector>
#include <iomanip>
#include <mutex>
#include <csignal> // Para signal e SIGINT

using namespace std;
std::mutex cout_mutex;
std::string current_input;
char input[100];
int n = 0;
bool type;
bool ctrl = 0;

void clearScreen() {
    std::cout << "\033[2J\033[H"; // Código de escape ANSI para limpar a tela
}

void drawHeader() {
    std::cout << std::endl;
    std::cout << "                                                                              *#&@@@@@@@%                                                     " << std::endl;
    std::cout << "                                                                           #@@@@@@@@@@@@&                                                     " << std::endl;
    std::cout << "                                                                         ,@@@@@@@@@@@@@@&                                                     " << std::endl;
    std::cout << "                           &@@@@@@@.  ,@@@%/      ,#@@@@@@@@@@@%*        @@@@@@@@@/                                                           " << std::endl;
    std::cout << "                           @@@@@@@@.  ,@@@@@@@. &@@@@@@@@@@@@@@@@@@.     @@@@@@@@@.                                                           " << std::endl;
    std::cout << "                           @@@@@@@@.  ,@@@@@@@@@@@@@@@@@@@@@@@@@@@@@/    @@@@@@@@@@@@@@@&                                                     " << std::endl;
    std::cout << "                           @@@@@@@@.  ,@@@@@@@@@@,        .@@@@@@@@@@.   @@@@@@@@@@@@@@@&                                                     " << std::endl;
    std::cout << "                           @@@@@@@@.  ,@@@@@@@@&            @@@@@@@@@,   @@@@@@@@@*,,,,,.                                                     " << std::endl;
    std::cout << "                           @@@@@@@@.  ,@@@@@@@@*            &@@@@@@@@*   @@@@@@@@@.                                                           " << std::endl;
    std::cout << "                           @@@@@@@@.  ,@@@@@@@@,            &@@@@@@@@*   @@@@@@@@@.                                                           " << std::endl;
    std::cout << "                           @@@@@@@@.  ,@@@@@@@@,            &@@@@@@@@*   @@@@@@@@@.                                                           " << std::endl;
    std::cout << "                           @@@@@@@@.  ,@@@@@@@@,            %&&&&&&&&*   &&&&&&&&&.                                                           " << std::endl;
    std::cout << "                 \033[31m*((*\033[0m      @@@@@@@@.  ,@@@@@@@@,      \033[30m...........................................\033[0m               " << std::endl;
    std::cout << "              \033[31m*########,\033[0m   @@@@@@@@.  ,@@@@@@@@,        \033[31mINF01151 - Sistemas Operacionais Ii N - Turma A (2024/1)\033[0m" << std::endl;
    std::cout << "              \033[31m(########/\033[0m   @@@@@@@@.  ,@@@@@@@@,        \033[31mTrabalho prático da disciplina\033[0m                          " << std::endl;
    std::cout << "               \033[31m/######*\033[0m    @@@@@@@@.  ,@@@@@@@@,        Arthur, Guilherme e Júlia                                        " << std::endl;
    std::cout << std::endl;
    std::cout << "                                                         _       __      __              ____              __                             " << std::endl;
    std::cout << "                                                        | |     / /___ _/ /_____        / __ \\____        / /   ____ _____               " << std::endl;
    std::cout << "                                                        | | /| / / __ `/ //_/ _ \\______/ / / / __ \\______/ /   / __ `/ __ \\            " << std::endl;
    std::cout << "                                                        | |/ |/ / /_/ / ,< /  __/_____/ /_/ / / / /_____/ /___/ /_/ / / / /               " << std::endl;
    std::cout << "                                                        |__/|__/\\__,_/_/|_|\\___/      \\____/_/ /_/     /_____/\\__,_/_/ /_/            " << std::endl;
    std::cout << "                                                                              __           __                                             " << std::endl;
    std::cout << "                                                            ____  ____ ______/ /____     <  /                                             " << std::endl;
    std::cout << "                                                           / __ \\/ __ `/ ___/ __/ _ \\    / /                                            " << std::endl;
    std::cout << "                                                          / /_/ / /_/ / /  / /_/  __/   / /                                               " << std::endl;
    std::cout << "                                                         / .___/\\__,_/_/   \\__/\\___/   /_/                                             " << std::endl;
    std::cout << "                                                        /_/                                                                               " << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    }

void drawInterface(){
    clearScreen(); 
    drawHeader();
}

// Função para configurar o terminal para ler uma tecla sem esperar por '\n'
void setTermNoBufferedInput() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag &= ~(ICANON | ECHO); // Desliga modo canônico e echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

// Função para restaurar as configurações originais do terminal
void restoreTermSettings() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);
    t.c_lflag |= (ICANON | ECHO); // Liga modo canônico e echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
}

void drawTableHeader() {
    std::cout << std::endl;
    std::cout << " ___________________________________________________________" << std::endl;
    std::cout << "|              |                   |               |        |" << std::endl;
    std::cout << "|   Hostname   |   Endereço MAC    |  Endereço IP  | Status |" << std::endl;
    std::cout << "|______________|___________________|_______________|________|" << std::endl;
}

void drawTableData(Server &server) {
    for (const auto &client : server.discoveredClients) {
        std::cout << "|              |                   |               |        |" << std::endl;
        std::cout << "| " << std::setw(12) << client.hostname
                << " | " << std::setw(17) << client.macAddress
                << " | " << std::setw(13) << client.ipAddress
                << " | " << std::setw(6) << client.status
                << " |" << std::endl;
        std::cout << "|______________|___________________|_______________|________|" << std::endl;
    }
}

void drawTable(Server &server) {
    drawTableHeader();
    drawTableData(server);
}


// =============

void clear_line() {
    std::cout << "\33[2K\r";
}

void read_input(Client &client, Server &server);

void manipulateInput(char input[100], Client &client, Server &server){
    std::string word(input);
    bool startsWithWake = (word.length() >= 4 && word.substr(0, 4) == "WAKE");
    if (word == "EXIT") {
        std::cout << client.managerInfo.hostname << ": saindo do sistema..." << std::endl;
        client.sendExitRequest(BROADCAST_ADDR);
        restoreTermSettings();
        std::exit(EXIT_SUCCESS);
    } else if (startsWithWake && type == 1) {
        word.erase(0, 5);
        sendWoLPacket(server, word);
    } else {
        std::cout << "Comando inválido!" << std::endl;
    }
    read_input(client, server);
}

void read_input(Client &client, Server &server) {
    setTermNoBufferedInput(); // Configura terminal para entrada sem buffer
    n=0;
    char ch;
    for (int i = 0; i < 100; ++i) {
        input[i] = '\0';
    }
    while (true) {
        if (read(STDIN_FILENO, &ch, 1) == 1) { 
            if (ch == '\b') { 
                if (n > 0) {
                    cout << "aaaa" << endl;
                    n--;
                    input[n] = '\0';
                }
            } else if (ch == '\n') { 
                manipulateInput(input,client, server);
                break; 
            } else {
                input[n++] = ch; 
                input[n] = '\0'; 
            }
            std::cout << "\033[1F"; // Move o cursor para cima em uma linha
            std::cout << "\033[2K"; // Limpa a linha atual
            cout << string(input) << endl;
        }
    }
}

void handleSigInt(int signum) {
    ctrl = 1;
}

void isCTRLc() {
    signal(SIGINT, handleSigInt);
}

void isCTRLcT(Client &client) {
    while (true) {
        if(ctrl) {
            std::cout << client.managerInfo.hostname << ": saindo do sistema..." << std::endl;
            sendExitRequest(client);
            restoreTermSettings();
            std::exit(EXIT_SUCCESS); 
        }
    }
}