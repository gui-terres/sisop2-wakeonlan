#ifndef DISCOVERY_H
#define DISCOVERY_H

#include "../../stations/stations.hpp"

class Discovery {
public:
    static void discoverParticipants(Server &manager);
    static void searchForManager(Client &client, int argc);
};

#endif // DISCOVERY_H
