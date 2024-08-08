#ifndef DISCOVERY_H
#define DISCOVERY_H

#include "../../stations/stations.hpp"
#include <cstring>
#include <iostream>
#include <thread>
#include <chrono>

class Discovery {
public:
    static void discoverParticipants(Server &manager);
    static void searchForManager(Client &client, int argc);
    static void enterWakeOnLan(Client &client, int argc);
};

#endif // DISCOVERY_H
