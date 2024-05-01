#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <string>

#define MAC_ADDRESS_SIZE 18

using std::string;

namespace {
    using discoveredData = struct {
        std::string hostname;
        std::string ipAddress;
        char macAddress[MAC_ADDRESS_SIZE];
    };
}

namespace Server {
    int socket();
}

namespace Client {
    int socket(int argc, const char *leaderHostname);
}

#endif // DISCOVERY_H