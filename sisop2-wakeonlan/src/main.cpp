#include <iostream>
#include "subsystems/discovery/discovery.hpp"

using namespace std;

int main(int argc, char **argv) {
    switch(argc) {
        case 1: {
            cout << "Manager mode" << endl;
            Server server;
            server.sendSocket();

            return 0;
        };
        case 2: {
            cout << "Client mode" << endl;
            Client client;
            client.sendSocket(argc, argv[1]);

            return 0;
        }
        default: cout << "Invalid initialiaztion!" << endl;
    }
}
