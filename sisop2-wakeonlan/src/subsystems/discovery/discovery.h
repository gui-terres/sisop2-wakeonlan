#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <string>

using std::string;

namespace {
    using discoveredData = struct {
        std::string hostname;
    };
}

namespace Server {
    int socket();
}

namespace Client {
    int socket();
}

#endif // DISCOVERY_H