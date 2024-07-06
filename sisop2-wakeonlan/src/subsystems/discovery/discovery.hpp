#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <string>

#define SERVER_PORT 4000
#define MAX_HOSTNAME_SIZE 250
#define IP_ADDRESS_SIZE 16
#define MAC_ADDRESS_SIZE 18

using std::string;

namespace {
    enum Status {
        ASLEEP,
        AWAKEN
    };

    using DiscoveredData = struct {
        char hostname[MAX_HOSTNAME_SIZE];
        char ipAddress[IP_ADDRESS_SIZE];
        char macAddress[MAC_ADDRESS_SIZE];
        Status status;
    };
}

class Server {
    public:
        int sendSocket();
};

class Client {
    private:
        int getHostname(char *buffer, size_t bufferSize, DiscoveredData &hostname);
        int getIpAddress(DiscoveredData &data);
        int getMacAddress(int sockfd, char *macAddress, size_t size);
        int getStatus(Status &status);
    public:
        int sendSocket(int argc, const char *leaderHostname);
};

#endif // DISCOVERY_H