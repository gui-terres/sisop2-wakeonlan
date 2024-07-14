#ifndef DISCOVERY_H
#define DISCOVERY_H

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

using std::string;

enum Status
{
    ASLEEP,
    AWAKEN
};

enum Request
{
    SLEEP_STATUS,
    EXIT,
    PARTICIPANT_DATA
};

struct DiscoveredData
{
    char hostname[MAX_HOSTNAME_SIZE];
    char ipAddress[IP_ADDRESS_SIZE];
    char macAddress[MAC_ADDRESS_SIZE];
    Status status;
};

struct RequestData
{
    Request request;
};

class Station {
public:
    int getHostname(char *buffer, size_t bufferSize, DiscoveredData &hostname);
    int getIpAddress(DiscoveredData &data);
    int getMacAddress(int sockfd, char *macAddress, size_t size);
    int getStatus(Status &status);
};

class Server : public Station {
public:
    int sendSocket(const char* addr);
    int requestSleepStatus(const char *ipAddress, RequestData request, Status &status);
    std::vector<DiscoveredData> getDiscoveredClients(); // Função para retornar a lista de clientes descobertos
    int sendWoLPacket(DiscoveredData &client);
    void waitForRequests(Server &server);
    DiscoveredData* requestParticipantData(const char *ipAddress);
};

class Client : public Station {
private:
    // 
public:
    DiscoveredData managerInfo;

    int sendSocket(int argc, Status status);
    void waitForRequests(Status status);
    int sendExitRequest(const char *ipAddress);
    void waitForParticipantDataRequests();
};

extern std::vector<DiscoveredData> discoveredClients;
extern std::mutex mtx;

#endif // DISCOVERY_H
