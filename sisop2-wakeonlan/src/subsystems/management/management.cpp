#include "management.hpp"

using namespace std;

std::mutex cout_mutex;

void Management::displayServer(Server &server) {
    while (!stopThreads.load()) {
        Monitoring::requestParticipantsSleepStatus(server);
        std::lock_guard<std::mutex> lock(cout_mutex);
        drawInterface();
        cout << "Para sair, digite EXIT" << endl;
        cout << "Para acordar um computador, digite WAKE + hostname" << endl;

        if (!discoveredClients.empty())
            drawTable(discoveredClients);
        else
          cout << endl << "Sem clientes no momento!" << endl << endl << endl;
        tempo();
        cout << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void Management::tempo(){
    std::time_t now = std::time(nullptr);
    // Converte o tempo para uma string legível
    std::tm* localTime = std::localtime(&now);
    // Imprime a data e hora atual
    std::cout << "Data e hora atual: " << std::asctime(localTime);
}

void Management::displayClient(Client &client) {
    while (!stopThreads.load()){
        //checar timeout aqui        
        if (!strcmp(managerInfo.ipAddress, PLACEHOLDER) || !strcmp(managerInfo.ipAddress, "")) {            
            cout << "Nenhum líder na rede" << endl;
        } else{
            cout << "Hostname: " << managerInfo.hostname << endl;
            cout << "IP Address: " << managerInfo.ipAddress << endl;
            cout << "Mac Address: " << managerInfo.macAddress << endl;
        }

        if (!discoveredClients.empty())
            drawTable(discoveredClients);
        else
          cout << endl << "Sem outros clientes no momento!" << endl << endl << endl;
        tempo();
        cout << endl;
        // clearScreen();
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void Management::checkAndElectClient(Client &client) {
    // while (!stopThreads.load()) {
    //     // Check if there is no leader in the network
    //     if (!strcmp(client.managerInfo.ipAddress, PLACEHOLDER) || !strcmp(client.managerInfo.ipAddress, "")) {
    //         cout << "Nenhum líder na rede. Iniciando eleição..." << endl;
    //         // cout << type << endl;
    //         // client.startElection();

    //     } 
    //     cout << endl;
    //     this_thread::sleep_for(chrono::seconds(1));
    // }
}