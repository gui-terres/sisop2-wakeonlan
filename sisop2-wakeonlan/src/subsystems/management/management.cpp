#include "management.hpp"

using namespace std;

std::mutex cout_mutex;

void Management::displayServer(Server &server) {
    while (true) {
        Monitoring::requestParticipantsSleepStatus(server);
        std::lock_guard<std::mutex> lock(cout_mutex);
        // drawInterface();
        cout << "Para sair, digite EXIT" << endl;
        cout << "Para acordar um computador, digite WAKE + hostname" << endl;

        if (!server.discoveredClients.empty())
            drawTable(server);
        else
          cout << endl << "Sem clientes no momento!" << endl << endl << endl;

        cout << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
}

void Management::displayClient(Client &client) {
    while (true){
        //checar timeout aqui
        if (!strcmp(client.managerInfo.ipAddress, PLACEHOLDER) || !strcmp(client.managerInfo.ipAddress, "")) {
            // std::cout << "clientetetete oiii server??????" << std::endl;
            cout << "Nenhum líder na rede" << endl;
        } else{
            cout << "Hostname: " << client.managerInfo.hostname << endl;
            cout << "IP Address: " << client.managerInfo.ipAddress << endl;
            cout << "Mac Address: " << client.managerInfo.macAddress << endl;
        }

        cout << endl;
        this_thread::sleep_for(chrono::seconds(1));
    }
}


