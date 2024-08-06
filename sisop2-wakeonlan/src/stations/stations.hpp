#ifndef STATIONS_H
#define STATIONS_H

#include <string>
#include <vector>
#include <mutex>

#define PORT 5000
#define PORT_S 5001
#define PORT_E 5002
#define PORT_PD 5003
#define MAX_HOSTNAME_SIZE 250
#define IP_ADDRESS_SIZE 16
#define MAC_ADDRESS_SIZE 18
#define BROADCAST_ADDR "255.255.255.255"
#define PLACEHOLDER "@@@@"

using std::string;

enum Status {
    ASLEEP,
    AWAKEN
};

enum Request {
    SLEEP_STATUS,
    EXIT,
    PARTICIPANT_DATA
};

struct StationData {
    char hostname[MAX_HOSTNAME_SIZE];
    char ipAddress[IP_ADDRESS_SIZE];
    char macAddress[MAC_ADDRESS_SIZE];
    Status status;
};

struct RequestData {
    Request request;
};

class Station {
public:
    int getHostname(char *buffer, size_t bufferSize, StationData &hostname);
    int getIpAddress(StationData &data);
    int getMacAddress(int sockfd, char *macAddress, size_t size);
    int getStatus(Status &status);
};

class Server : public Station {
public:
    std::vector<StationData> discoveredClients;

    int sendSocket(const char* addr);
    int requestSleepStatus(const char *ipAddress, RequestData request, Status &status);
    std::vector<StationData> getDiscoveredClients();
    int sendWoLPacket(StationData &client);
    void waitForRequests();
    StationData* requestParticipantData(const char *ipAddress);
};

class Client : public Station {
public:
    StationData managerInfo = {
        hostname: PLACEHOLDER,
        ipAddress: PLACEHOLDER,
        macAddress: PLACEHOLDER,
        status: Status::ASLEEP
    };

    int sendSocket(int argc);
    void waitForRequests();
    int sendExitRequest(const char *ipAddress);
    void waitForParticipantDataRequests();
};

extern std::mutex mtx;

#endif // STATIONS_H
