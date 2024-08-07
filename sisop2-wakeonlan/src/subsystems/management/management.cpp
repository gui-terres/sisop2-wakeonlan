#include "management.hpp"

using namespace std;

std::mutex cout_mutex;

void Management::display(Server &server) {
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

