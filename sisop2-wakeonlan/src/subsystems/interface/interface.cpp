#include "interface.hpp"

#include <iostream>
#include <cstdlib>
#include <string.h>
#include <thread>
#include <vector>
#include <iomanip>
#include <mutex>
#include <csignal> // Para signal e SIGINT

using namespace std;

std::string current_input;
// Type type;
char input[100];
int n = 0;
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
    std::cout << "                                                         _       __      __              ____              __                                 " << std::endl;
    std::cout << "                                                        | |     / /___ _/ /_____        / __ \\____        / /   ____ _____                   " << std::endl;
    std::cout << "                                                        | | /| / / __ `/ //_/ _ \\______/ / / / __ \\______/ /   / __ `/ __ \\                " << std::endl;
    std::cout << "                                                        | |/ |/ / /_/ / ,< /  __/_____/ /_/ / / / /_____/ /___/ /_/ / / / /                   " << std::endl;
    std::cout << "                                                        |__/|__/\\__,_/_/|_|\\___/      \\____/_/ /_/     /_____/\\__,_/_/ /_/                " << std::endl;
    std::cout << "                                                            ____  ____ ______/ /____     |__ \\                                               " << std::endl;
    std::cout << "                                                           / __ \\/ __ `/ ___/ __/ _ \\    __/ /                                              " << std::endl;
    std::cout << "                                                          / /_/ / /_/ / /  / /_/  __/   / __/                                                 " << std::endl;
    std::cout << "                                                         / .___/\\__,_/_/   \\__/\\___/   /____/                                              " << std::endl;
    std::cout << "                                                        /_/                                                                                   " << std::endl; 
    std::cout << std::endl;
    std::cout << std::endl;

    if (type == Type::MANAGER){
        std::cout << " ███    ███  █████  ███    ██  █████   ██████  ███████ ██████  " << std::endl; 
        std::cout << " ████  ████ ██   ██ ████   ██ ██   ██ ██       ██      ██   ██ " << std::endl; 
        std::cout << " ██ ████ ██ ███████ ██ ██  ██ ███████ ██   ███ █████   ██████  " << std::endl; 
        std::cout << " ██  ██  ██ ██   ██ ██  ██ ██ ██   ██ ██    ██ ██      ██   ██ " << std::endl; 
        std::cout << " ██      ██ ██   ██ ██   ████ ██   ██  ██████  ███████ ██   ██ " << std::endl;     
    }
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
    std::cout << " _________________________________________________________________________________" << std::endl;
    std::cout << "|              |                   |               |        |                    |" << std::endl;
    std::cout << "|   Hostname   |   Endereço MAC    |  Endereço IP  | Status |       Tipo         |" << std::endl;
    std::cout << "|______________|___________________|_______________|________|____________________|" << std::endl;
}

std::string typeToString(Type type) {
    switch (type) {
        case PARTICIPANT:    return "PARTICIPANT";
        case MANAGER:        return "MANAGER";
        case SLEEPY_MANAGER: return "SLEEPY_MANAGER";
        default:             return "UNKNOWN";
    }
}

void drawTableData(const std::vector<StationData>& discoveredClients) {
    bool managerPrinted = false;

    for (const auto& client : discoveredClients) {
        if (client.type == Type::MANAGER && !managerPrinted) {
            std::cout << "|              |                   |               |        |                    |" << std::endl;
            std::cout << "| " << std::setw(12) << client.hostname
                     << " | " << std::setw(17) << client.macAddress
                     << " | " << std::setw(13) << client.ipAddress
                     << " | " << std::setw(6) << (client.status == Status::AWAKEN ? "AWAKEN" : "ASLEEP")
                     << " | " << std::setw(18) << typeToString(client.type)
                     << " |"  << std::endl;
            std::cout << "|______________|___________________|_______________|________|____________________|" << std::endl;
            managerPrinted = true;
        }
    }

    // Imprime os demais clientes
    for (const auto& client : discoveredClients) {
        if (client.type != Type::MANAGER) {
            std::cout << "|              |                   |               |        |                    |" << std::endl;
            std::cout << "| " << std::setw(12) << client.hostname
                     << " | " << std::setw(17) << client.macAddress
                     << " | " << std::setw(13) << client.ipAddress
                     << " | " << std::setw(6) << (client.status == Status::AWAKEN ? "AWAKEN" : "ASLEEP")
                     << " | " << std::setw(18) << typeToString(client.type)
                     << " |"  << std::endl;
            std::cout << "|______________|___________________|_______________|________|____________________|" << std::endl;
        }
    }
}

void drawTable(const std::vector<StationData>& discoveredClients) {
    drawTableHeader();
    drawTableData(discoveredClients);
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
        std::cout << managerInfo.hostname << ": saindo do sistema..." << std::endl;
        if (type == Type::PARTICIPANT)
            client.sendExitRequest(managerInfo.ipAddress);
        restoreTermSettings();
        std::exit(EXIT_SUCCESS);
    } else if (startsWithWake && type == Type::MANAGER ) {
        word.erase(0, 5);
        cout << "Mandando WakeOnLan para " << word << endl;
        Monitoring::sendWoLPacket(server, word);
    } else {
        std::cout << "Comando inválido!" << std::endl;
    }
    read_input(client, server);
}

void read_input(Client &client, Server &server) {
    setTermNoBufferedInput(); // Configura terminal para entrada sem buffer
    n = 0;
    char ch;
    std::string inputStr;

    while (!stopThreads.load()) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        // Configura o tempo de espera para a função select
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500000; // Tempo de espera de 0.5 segundos

        // Verifica se há dados disponíveis para leitura
        int ret = select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &tv);

        if (ret == -1) {
            // std:: cerr << "Erro na função select" << std::endl;
            continue;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (read(STDIN_FILENO, &ch, 1) == 1) { 
                if (ch == '\b') { 
                    if (n > 0) {
                        n--;
                        input[n] = '\0';
                    }
                } else if (ch == '\n') { 
                    inputStr = std::string(input);
                    manipulateInput(input, client, server);
                    // Limpa o input para o próximo comando
                    std::fill(std::begin(input), std::end(input), '\0');
                    n = 0;
                } else {
                    input[n++] = ch; 
                    input[n] = '\0'; 
                }
                std::cout << "\033[1F"; // Move o cursor para cima em uma linha
                std::cout << "\033[2K"; // Limpa a linha atual
                std::cout << input << std::endl;
            }
        }
    }

    restoreTermSettings();        
}

void handleSigInt(int signum) {
    ctrl = 1;
}

void isCTRLc() {
    signal(SIGINT, handleSigInt);
}

void isCTRLcT(Client &client) {
    while (!stopThreads.load()) {
        if(ctrl) {
            if (type == Type::PARTICIPANT){
                std::cout << managerInfo.hostname << ": saindo do sistema..." << std::endl;            
                client.sendExitRequest(managerInfo.ipAddress);
            }
            restoreTermSettings();
            std::exit(EXIT_SUCCESS); 
        }
    }
}