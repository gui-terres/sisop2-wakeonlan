#include "./monitoring.hpp"
#include "../../stations/stations.hpp"

using namespace std;

void Monitoring::requestParticipantsSleepStatus(Server &manager) {
    RequestData req;
    req.request = Request::SLEEP_STATUS;
    StationData pc;
    manager.getIpAddress(pc);

    for (StationData &client : discoveredClients) { // Pode modificar
        Status status;
        if (!strcmp(client.ipAddress,pc.ipAddress)){
            // cout << "eu mesmo" << endl;
            client.status = Status::AWAKEN;
            continue;
        }
        int result = manager.requestSleepStatus(client.ipAddress, req, status);
        // std::cout << "Request result for " << client.ipAddress << ": " << result << std::endl;
        if (result == 0) {
            client.status = status; // Modifica a lista original
            // std::cout << "Client status updated: " << client.status << std::endl;
        } else {
            // std::cout << "Failed to request sleep status for " << client.ipAddress << std::endl;
        }
    }
}

void Monitoring::sendDowngradeToSleepyManagers(Server &manager) {
    RequestData req;
    req.request = Request::DOWNGRADE;
    StationData pc;
    manager.getIpAddress(pc);

    while (!stopThreads.load()){
        for (StationData &client : discoveredClients) { // Pode modificar
            if (client.type == Type::SLEEPY_MANAGER){
                // cout << "Mandando downgrade para " << client.ipAddress << "..." << endl;
                manager.sendRequest(PORT_DOWNGRADE, client.ipAddress, req);
            }
        }
        this_thread::sleep_for(chrono::milliseconds(200));
    }
}

void Monitoring::waitForSleepStatusRequest(Client &client) {
    client.waitForSleepRequests();
}

void Monitoring::sendWoLPacket(Server &server, string hostname) {
    char* cstr = new char[hostname.length() + 1];
    strcpy(cstr, hostname.c_str());
    for (StationData &client : discoveredClients) {
        if (!strcmp(client.hostname, cstr)) {  // Argument of command WAKEUP hostname
            server.sendWoLPacket(client);
            return;
        }
    }
    
    cout << "Participante com o hostname não encontrado ou não está dormindo." << endl;
}