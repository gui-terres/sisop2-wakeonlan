#include "management.hpp"
#include "../interface/interface.hpp"
#include "../monitoring/monitoring.hpp"

static void Management::display(Server &server) {
    while (true) {
        Monitoring.requestParticipantsSleepStatus(server);
        std::lock_guard<std::mutex> lock(cout_mutex);
        drawInterface();
        cout << "Para sair, digite EXIT" << endl;
        cout << "Para acordar um computador, digite WAKE + endereço IP" << endl;

        if (!discoveredClients.empty())
            drawTable(server);
        else
          cout << endl << "Sem clientes no momento!" << endl << endl << endl;

        cout << endl;
        this_thread::sleep_for(chrono::seconds(5));
    }
}

