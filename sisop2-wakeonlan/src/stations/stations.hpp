#ifndef STATIONS_H
#define STATIONS_H

#include <string>
#include <vector>
#include <mutex>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <condition_variable>
#include <atomic>

#define PORT_SOCKET 55000
#define PORT_SLEEP 55001
#define PORT_EXIT 55002
#define PORT_MANAGER_DATA 55003
#define PORT_ELECTION 55004
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

extern Type type;

enum Request {
    SLEEP_STATUS,
    EXIT,
    PARTICIPANT_DATA
};

enum MessageType {
    ELECTION,
    OK,
    COORDINATOR
};

struct Message {
    MessageType type;
    int id;
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
    // std::vector<std::string> stationIPs;

    int getHostname(char *buffer, size_t bufferSize, StationData &hostname);
    int getIpAddress(StationData &data);
    int getMacAddress(int sockfd, char *macAddress, size_t size);
    int getStatus(Status &status);
    int createSocket(int port = 0);
    void setSocketBroadcastOptions(int sockfd);
    void sendMessage(const string& destIP, Message msg, int port);
    Message receiveMessage(int port);

    // New methods to support the election process
    void startElection();
    void sendCoordinatorMessage();

    static std::vector<std::string> stationIPs;

private:
    int id;
    bool isCoordinator;
};

class Server: public Station {
public:
    std::vector<StationData> discoveredClients;

    int collectParticipants(const char* addr);
    int requestSleepStatus(const char *ipAddress, RequestData request, Status &status);
    std::vector<StationData>& getDiscoveredClients();
    int sendWoLPacket(StationData &client);
    void waitForRequests();
    int sendManagerInfo();
    void startElection(); // Start election from the server's perspective

    void updateStationIPs();
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
    void waitForSleepRequests();
    int sendExitRequest(const char *ipAddress);
    int getManagerData();
    void startElection(); // Start election from the client's perspective
};

extern std::mutex mtx;
extern std::condition_variable cv;
extern std::atomic<bool> stopThreads;

#endif // STATIONS_H
