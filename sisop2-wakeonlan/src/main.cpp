#include <iostream>
#include "subsystems/discovery/discovery.h"

using namespace std;

int main(int argc, char **argv) {
    switch(argc) {
        case 1: {
            cout << "Leader mode" << endl;
            Server::socket();
            break;
        };
        case 2: {
            cout << "Client mode" << endl;
            Client::socket(argc, argv[1]);
            break;
        }
        default: cout << "Invalid initialiaztion!" << endl;
    }

    return 0;
}