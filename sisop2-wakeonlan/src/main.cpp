#include <iostream>
#include <string.h>
#include <thread>
#include <vector>
#include "subsystems/discovery/discovery.hpp"

using namespace std;

void runSendSocket(Server &server)
{
    server.sendSocket();
}

void requestParticipantsSleepStatus(Server &server)
{
    while (true)
    {
        this_thread::sleep_for(chrono::seconds(5));
        RequestData req;
        req.request = Request::SLEEP_STATUS;

        for (const auto &client : discoveredClients)
        {
            Status status;
            if (server.requestSleepStatus(client.ipAddress, req, status) == 0)
            {
                cout << "Status do cliente " << client.ipAddress << ": " << (status == AWAKEN ? "AWAKEN" : "ASLEEP") << endl;
            }
        }
    }
}

// GERENCIAMENTO - sem WakeOnLan
void displayDiscoveredClients(Server &server)
{
    while (true)
    {
        this_thread::sleep_for(chrono::seconds(5));

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

void searchForManager(Client &client, int argc, Status status)
{
    client.sendSocket(argc, "s-67-101-12", status);
}

void waitForRequests(Client &client, Status status)
{
    client.waitForRequests(status);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        cout << "Invalid initialization!" << endl;
        return 1;
    }

    if (strcmp(argv[1], "1") == 0)
    {
        cout << "Manager mode" << endl;
        Server server;

        thread t1(runSendSocket, ref(server));
        thread t2(requestParticipantsSleepStatus, ref(server));
        thread t3(displayDiscoveredClients, ref(server));

        t1.join();
        t2.join();
        t3.join();

        return 0;
    }
    else if (strcmp(argv[1], "2") == 0)
    {
        cout << "Client mode" << endl;
        Client client;
        Status status;

        if (strcmp(argv[2], "AWAKEN") == 0)
        {
            status = Status::AWAKEN;
        }
        else
        {
            status = Status::ASLEEP;
        }

        thread t3(searchForManager, ref(client), argc, status);
        thread t4(waitForRequests, ref(client), status);

        t3.join();
        t4.join();

        return 0;
    }
    else
    {
        cout << "Invalid initialization!" << endl;
        return 1;
    }
}