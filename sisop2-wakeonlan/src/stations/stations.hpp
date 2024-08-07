#ifndef STATIONS_H
#define STATIONS_H

#include <string>
#include <vector>
#include <mutex>
#include <cstring>

#define PORT_SOCKET 5000
#define PORT_SLEEP 5001
#define PORT_EXIT 5002
#define PORT_DATA 5003
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

enum Type {
    PARTICIPANT,
    MANAGER
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
    Type type;
    Status status;

    // Definição do operador `==`
    bool operator==(const StationData& other) const {
        if (std::strcmp(ipAddress, other.ipAddress) != 0) {
            return false;
        }
        if (std::strcmp(macAddress, other.macAddress) != 0) {
            return false;
        }
        if (std::strcmp(hostname, other.hostname) != 0) {
            return false;
        }
        return true;
    }    
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
    int createSocket(int protocol = SOCK_DGRAM, int port = 0);
    void setSocketOptions(int sockfd);
    int sendData(int sockfd, const void* data, size_t dataSize, const char* addr, int port);
    ssize_t receiveData(int sockfd, void* buffer, size_t bufferSize, struct sockaddr_in* from, socklen_t* fromlen);
};

class Server: public Station {
public:
    std::vector<StationData> discoveredClients;

    int collectParticipants(const char* addr);
    int requestSleepStatus(const char *ipAddress, RequestData request, Status &status);
    std::vector<StationData>& getDiscoveredClients();
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

    int enterWakeOnLan(int argc);
    void waitForRequests();
    int sendExitRequest(const char *ipAddress);
    void waitForParticipantDataRequests();
};

extern std::mutex mtx;

#endif // STATIONS_H
