#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <vector>
#include <mutex>
#include <iomanip>
#include <vector>
#include <sstream>
#include <algorithm>

#include "stations.hpp"

#define BUFFER_SIZE 256

using namespace std;

// Mutex para sincronização de acesso à lista
std::mutex mtx;

void Server::updateStationIPs() {
    std::lock_guard<std::mutex> lock(mtx);
    stationIPs.clear();  // Clear the previous list

    for (const auto& client : discoveredClients) {
        stationIPs.push_back(std::string(client.ipAddress));
    }
}

int Server::collectParticipants(const char* addr = BROADCAST_ADDR) {
    int sockfd = createSocket(PORT_SOCKET);
    setSocketBroadcastOptions(sockfd);
    
    sockaddr_in cli_addr;
    socklen_t clilen = sizeof(struct sockaddr_in);

    while (!stopThreads.load()) {
        StationData receivedData;
        memset(&receivedData, 0, sizeof(receivedData));

        
        ssize_t bytesReceived = recvfrom(sockfd, &receivedData, sizeof(receivedData), 0, (struct sockaddr *)&cli_addr, &clilen);
        if (bytesReceived < 0) {
            cerr << "ERROR on recvfrom in collectParticipants." << endl;
            break;
        }

        {
            std::lock_guard<std::mutex> lock(mtx);

            // Verifica se o item já está na lista
            auto client = std::find(discoveredClients.begin(), discoveredClients.end(), receivedData);

            // // Debug: Imprime o status da lista
            // std::cout << "Verificando se o item já está na lista..." << std::endl;
            // for (const auto& c : discoveredClients) {
            //     std::cout << "Na lista: " << c.ipAddress << std::endl;
            // }

            // Se não encontrar o item, adiciona
            if (client == discoveredClients.end()) {
                // std::cout << "Item não encontrado, adicionando..." << std::endl;
                discoveredClients.push_back(receivedData);
            } else {
                // std::cout << "Item já existe na lista." << std::endl;
            }
            
            updateStationIPs();
        }

        // char buffer[BUFFER_SIZE];
        // StationData managerInfo;
        // memset(&managerInfo, 0, sizeof(managerInfo));

        // getHostname(buffer, BUFFER_SIZE, managerInfo);
        // getIpAddress(managerInfo);
        // getMacAddress(sockfd, managerInfo.macAddress, MAC_ADDRESS_SIZE);

        // if (sendto(sockfd, &managerInfo, sizeof(managerInfo), 0, (struct sockaddr *)&cli_addr, sizeof(struct sockaddr)) < 0) {
        //     cerr << "ERROR on sendto." << endl;
        // }
    }

    close(sockfd);
    return 0;
}

int Server::requestSleepStatus(const char *ipAddress, RequestData request, Status &status) {
    int sockfd = createSocket(PORT_SLEEP);

    // Definir tempo limite de 1 segundo para recebimento
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
        cerr << "ERROR setting socket timeout." << endl;
        close(sockfd);
        return -1;
    }

    struct sockaddr_in recipient_addr;
    memset(&recipient_addr, 0, sizeof(recipient_addr));
    recipient_addr.sin_family = AF_INET;
    recipient_addr.sin_port = htons(PORT_SLEEP);
    if (inet_pton(AF_INET, ipAddress, &recipient_addr.sin_addr) <= 0) {
        cerr << "ERROR invalid address/ Address not supported." << endl;
        close(sockfd);
        return -1;
    }

    if (sendto(sockfd, &request, sizeof(request), 0, (struct sockaddr *)&recipient_addr, sizeof(recipient_addr)) < 0) {
        cerr << "ERROR sending request." << endl;
        close(sockfd);
        return -1;
    }

    // Receber resposta
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    Status responseStatus = Status::AWAKEN;

    ssize_t bytesReceived = recvfrom(sockfd, &responseStatus, sizeof(responseStatus), 0, (struct sockaddr *)&from, &fromlen);
    if (bytesReceived < 0) {
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
            cerr << "ERROR: Timeout receiving response." << endl;
        } else {
            cerr << "ERROR receiving response." << endl;
        }
        status = Status::ASLEEP;
        close(sockfd);
        return 0;
    }

    status = responseStatus;
    close(sockfd);
    return 0;
}

int Server::sendManagerInfo() {
    int sockfd = createSocket();
    if (sockfd == -1) return 1;

    setSocketBroadcastOptions(sockfd);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_MANAGER_DATA);
    serv_addr.sin_addr.s_addr = inet_addr(BROADCAST_ADDR);
    bzero(&(serv_addr.sin_zero), 8);

    char buffer[BUFFER_SIZE];
    StationData pcData;
    getHostname(buffer, BUFFER_SIZE, pcData);
    getIpAddress(pcData);
    getMacAddress(sockfd, pcData.macAddress, MAC_ADDRESS_SIZE);
    pcData.status = Status::AWAKEN;

    // cout << "Manager Info" << endl;
    // cout << "Hostname: " << pcData.hostname << endl;
    // cout << "IP Address: " << pcData.ipAddress << endl;
    // cout << "Mac Address: " << pcData.macAddress << endl;
    while (!stopThreads.load()) {
        if (sendto(sockfd, &pcData, sizeof(pcData), 0, (const struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0){
            cerr << "ERROR on sendto." << endl;
            break;
        }
    }

    close(sockfd);
    return 0;
}

std::vector<StationData>& Server::getDiscoveredClients() {
    std::lock_guard<std::mutex> lock(mtx);
    return discoveredClients;
}

void assembleWoLPacket(std::vector<uint8_t> &packet, StationData &client);

int Server::sendWoLPacket(StationData &client) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        cerr << "ERROR opening socket." << endl;
        return -1;
    }

    struct sockaddr_in recipient_addr;
    memset(&recipient_addr, 0, sizeof(recipient_addr));
    recipient_addr.sin_family = AF_INET;
    recipient_addr.sin_port = htons(9);
    if (inet_pton(AF_INET, client.ipAddress, &recipient_addr.sin_addr) <= 0) {
        cerr << "ERROR invalid address/ Address not supported." << endl;
        close(sockfd);
        return -1;
    }

    std::vector<uint8_t> packet;
    assembleWoLPacket(packet, client);

    if (sendto(sockfd, packet.data(), packet.size(), 0, (struct sockaddr *)&recipient_addr, sizeof(recipient_addr)) < 0) {
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}

std::vector<uint8_t> macStringToBytes(const std::string &macAddress) {
    std::vector<uint8_t> bytes;
    std::istringstream iss(macAddress);
    std::string token;

    while (std::getline(iss, token, ':')) {
        bytes.push_back(std::stoul(token, nullptr, 16));
    }

    return bytes;
}

void assembleWoLPacket(std::vector<uint8_t> &packet, StationData &client) {
    for (int i = 0; i < 6; ++i) {
        packet.push_back(0xFF);
    }

    std::vector<uint8_t> macBytes = macStringToBytes(client.macAddress);
    for (int i = 0; i < 16; ++i) {
        packet.insert(packet.end(), macBytes.begin(), macBytes.end());
    }
}

void Server::waitForRequests() {
    int sockfd = createSocket(PORT_EXIT);

    while (!stopThreads.load()) {
        RequestData request;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        ssize_t bytesReceived = recvfrom(sockfd, &request, sizeof(request), 0, (struct sockaddr *)&from, &fromlen);
        if (bytesReceived < 0) {
            cerr << "ERROR on recvfrom waitForRequests." << endl;
            continue;
        }

        if (request.request == Request::EXIT) {
            char ipAddress[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &from.sin_addr, ipAddress, sizeof(ipAddress)) == nullptr) {
                cerr << "ERROR converting address." << endl;
                close(sockfd);                
            } else {
                this->discoveredClients.erase(
                    std::remove_if(this->discoveredClients.begin(), this->discoveredClients.end(),
                                [ipAddress](const StationData& data) {
                                    return strcmp(data.ipAddress, ipAddress) == 0;
                                }),
                    this->discoveredClients.end()
                );
            }

        }
    }

    close(sockfd);
}

void Server::startElection() {
    Station::startElection();
}