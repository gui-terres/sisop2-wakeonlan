#include <iostream>
#include "subsystems/discovery/discovery.h"

using namespace std;

int main(int argc, char **argv) {
    Client::socket(argc, argv[1]);

    return 0;
}