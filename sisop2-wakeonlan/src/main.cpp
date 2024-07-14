#include <iostream>
#include <string.h>
#include <thread>
#include <vector>
#include "subsystems/discovery/discovery.hpp"

using namespace std;

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
void displayDiscoveredClients(Server &server)
{
    while (true) {
        this_thread::sleep_for(chrono::seconds(5));

        if (!discoveredClients.empty()) {
            cout << "Printing Clients:" << endl;
            cout << "----------------------------" << endl;
            for (const auto &client : discoveredClients)
            {
                cout << "Hostname: " << client.hostname << endl;
                cout << "IP Address: " << client.ipAddress << endl;
                cout << "MAC Address: " << client.macAddress << endl;
                cout << "____________" << endl;
            }
            cout << "----------------------------" << endl;
        }
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

void sendExitRequest(Client &client)
{   
    this_thread::sleep_for(chrono::seconds(5));
    client.sendExitRequest("172.18.0.2");
}

void waitForParticipantDataRequests(Client &client)
{
    client.waitForParticipantDataRequests();
}

void waitForRequestsServer(Server &server)
{
    server.waitForRequests();
}

void requestParticipantData(Server &server)
{   
    while (true)
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

void runManagerMode() {
    cout << "Manager mode" << endl;
    Server server;

    thread t1(runSendSocket, ref(server));
    // thread t2(requestParticipantsSleepStatus, ref(server));
    // thread t3(displayDiscoveredClients, ref(server));
    // thread t4(sendWoLPacket, ref(server));
    thread t5(waitForRequestsServer, ref(server));
    thread t6(requestParticipantData, ref(server));

    t1.join();
    // t2.join();
    // t3.join();
    // t4.join();
    t5.join();
    t6.join();
}

void runClientMode(int argc) {
    cout << "Client mode" << endl;
    Client client;
    Status status = Status::AWAKEN;

    thread t3(searchForManager, ref(client), argc, status);
    thread t4(waitForRequestsClient, ref(client), status);
    thread t5(sendExitRequest, ref(client));
    thread t6(waitForParticipantDataRequests, ref(client));

    t3.join();
    t4.join();
    t5.join();
    t6.join();
}

int main(int argc, char **argv)
{
    if (argc > 2) {
        cout << "Invalid initialization!" << endl;
        return 1;
    }

    if (argc > 1) {
        if (!strcmp(argv[1], "manager")) {
            runManagerMode();
            return 0;
        } else {
            cout << "ERROR: Invalid argument initialization!" << endl;
            cout << argv[1] << " isn't a valid mode." << endl;
            return 1;
        }
    } else {
        runClientMode(argc);
        return 0;
    }

    // if (strcmp(argv[1], "1") == 0)
    // {
    //     cout << "Manager mode" << endl;
    //     Server server;

    //     thread t1(runSendSocket, ref(server));
    //     thread t2(requestParticipantsSleepStatus, ref(server));
    //     thread t3(displayDiscoveredClients, ref(server));
    //     // thread t4(sendWoLPacket, ref(server));

    //     t1.join();
    //     // t2.join();
    //     t3.join();
    //     // t4.join();

    //     return 0;
    // }
    // else if (strcmp(argv[1], "2") == 0)
    // {
    //     cout << "Client mode" << endl;
    //     Client client;
    //     Status status;

    //     if (strcmp(argv[2], "AWAKEN") == 0)
    //     {
    //         status = Status::AWAKEN;
    //     }
    //     else
    //     {
    //         status = Status::ASLEEP;
    //     }

    //     thread t3(searchForManager, ref(client), argc, status);
    //     thread t4(waitForRequests, ref(client), status);

    //     t3.join();
    //     t4.join();

    //     return 0;
    // }
    // else
    // {
    //     cout << "Invalid initialization!" << endl;
    //     return 1;
    // }
}