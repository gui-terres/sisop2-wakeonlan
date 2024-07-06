#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <string>

#define SERVER_PORT 4000
#define MAC_ADDRESS_SIZE 18

using std::string;

namespace {
    enum Status {
        ASLEEP,
        AWAKEN
    };

    using DiscoveredData = struct {
        std::string hostname;
        std::string ipAddress;
        char macAddress[MAC_ADDRESS_SIZE];
        // Status status;
    };

    Status intToStatus(int value) {
        switch(value) {
            case 0:
                return Status::ASLEEP;
            default:
                return Status::AWAKEN;
        }
    }
}

class Server {
    public:
        int sendSocket();
};

class Client {
    private:
        int getHostname(char *buffer, size_t bufferSize, string &hostname);
        int getIpAddress(string &ipAddress);
        int getMacAddress(int sockfd, char *macAddress, size_t size);
        int getStatus(Status &status);
    public:
        int sendSocket(int argc, const char *leaderHostname);
};

#endif // DISCOVERY_H