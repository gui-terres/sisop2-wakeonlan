#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <string>
#include <vector>
#include <mutex>

#define PORT 5000
#define PORT_S 5001
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
    SLEEP_STATUS
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

class Server
{
public:
    int sendSocket(const char* addr);
    int requestSleepStatus(const char *hostname, RequestData request, Status &status);
    std::vector<DiscoveredData> getDiscoveredClients(); // Função para retornar a lista de clientes descobertos
    int sendWoLPacket(DiscoveredData &client);
};

class Client
{
private:
    int getHostname(char *buffer, size_t bufferSize, DiscoveredData &hostname);
    int getIpAddress(DiscoveredData &data);
    int getMacAddress(int sockfd, char *macAddress, size_t size);
    int getStatus(Status &status);

public:
    int sendSocket(int argc, Status status);
    void waitForRequests(Status status);
};

extern std::vector<DiscoveredData> discoveredClients;
extern std::mutex mtx;

#endif // DISCOVERY_H
