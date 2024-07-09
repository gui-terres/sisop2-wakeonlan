#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <string>
#include <vector>
#include <mutex>

#define PORT 55001
#define PORT_S 55002
#define MAX_HOSTNAME_SIZE 250
#define IP_ADDRESS_SIZE 16
#define MAC_ADDRESS_SIZE 18

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
    int sendSocket();
    int requestSleepStatus(const char *hostname, RequestData request, Status &status);
    std::vector<DiscoveredData> getDiscoveredClients(); // Função para retornar a lista de clientes descobertos
};

class Client
{
private:
    int getHostname(char *buffer, size_t bufferSize, DiscoveredData &hostname);
    int getIpAddress(DiscoveredData &data);
    int getMacAddress(int sockfd, char *macAddress, size_t size);
    int getStatus(Status &status);

public:
    int sendSocket(int argc, const char *leaderHostname);
    void waitForRequests();
};

extern std::vector<DiscoveredData> discoveredClients;
extern std::mutex mtx;

#endif // DISCOVERY_H
