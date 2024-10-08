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
#include <netinet/in.h> 
#include <chrono>  // Para std::this_thread::sleep_for

#define PORT_SOCKET 55000
#define PORT_SLEEP 55001
#define PORT_EXIT 55002
#define PORT_MANAGER_DATA 55003
#define PORT_ELECTION 55004
#define PORT_COORDINATOR 55005
#define PORT_TABLE 55006
#define PORT_ELECTION_RESPONSE 55007
#define PORT_DOWNGRADE 55008
#define MAX_HOSTNAME_SIZE 250
#define IP_ADDRESS_SIZE 16
#define MAC_ADDRESS_SIZE 18
#define BUFFER_SIZE 256
#define BROADCAST_ADDR "255.255.255.255"
#define PLACEHOLDER "@@@@"

using std::string;

enum Status {
    ASLEEP,
    AWAKEN
};

enum Type {
    PARTICIPANT,
    MANAGER,
    SLEEPY_MANAGER
};

extern Type type;
extern int id;

enum Request {
    SLEEP_STATUS,
    EXIT,
    PARTICIPANT_DATA,
    DOWNGRADE,
};

enum MessageType {
    ELECTION,
    OK,
    COORDINATOR
};

struct ManagerDuplicate {
    int id;
    int generation;
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
        if (type != other.type) {
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
    int getLastFieldOfIP(const char* ip);
    void setSocketBroadcastOptions(int sockfd);
    void setSocketReuseOptions(int sockfd);
    void setSocketTimeout(int sockfd, int timeoutSec);
    void startElection();
    void sendMessage(int port, int sockfd, const StationData& client, const Message& message); 
    bool waitForOkMessage(int sockfd);
    void sendCoordinatorMessage();
    void listenForElectionMessages();
    void sendOkResponse(const sockaddr_in& senderAddr);
    int requestSleepStatus(const char *ipAddress, RequestData request, Status &status);
    void waitForSleepRequests();
    void tempo();
    // static std::vector<std::string> stationIPs;
};

class Server: public Station {
public:
    int collectParticipants(const char* addr);
    std::vector<StationData>& getDiscoveredClients();
    int sendWoLPacket(StationData &client);
    void waitForRequests();
    int sendManagerInfo();
    void listenOnPort(int port);
    //void receiveMessages();
    // StationData* requestParticipantData(const char *ipAddress);
    void sendTable();
    int sendRequest(int port, const char *ipAddress, RequestData request);
};

class Client : public Station {
public:
    int enterWakeOnLan(int argc);
    int sendExitRequest(const char *ipAddress);
    int getManagerData();
    //void sendMessage(const std::string &message, const std::string &ipAddress);
    void askForTable();
};

extern std::mutex mtx;
extern std::condition_variable cv;
extern std::atomic<bool> stopThreads;
extern std::vector<StationData> discoveredClients;
extern StationData managerInfo;

#endif // STATIONS_H
