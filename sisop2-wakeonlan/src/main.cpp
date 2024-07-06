#include <iostream>
#include <string.h>
#include "subsystems/discovery/discovery.hpp"

using namespace std;

int main(int argc, char **argv) {
     if (argc < 2) {
        cout << "Invalid initialization!" << endl;
        return 1;
    }

    // Comparando diretamente o argumento de linha de comando
    if (strcmp(argv[1], "1") == 0) {
        cout << "Manager mode" << endl;
        Server server;
        server.sendSocket();

        return 0;
    } else if (strcmp(argv[1], "2") == 0) {
        cout << "Client mode" << endl;
        Client client;
        client.sendSocket(argc, "manager");

        return 0;
    } else {
        cout << "Invalid initialization!" << endl;
        return 1;
    }
}
