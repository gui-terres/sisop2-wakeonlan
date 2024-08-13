#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

#include "./stations.hpp"

#define BUFFER_SIZE 256

using namespace std;

int Station::getHostname(char *buffer, size_t bufferSize, StationData &data)
{
    memset(buffer, 0, sizeof(buffer));

    if ((gethostname(buffer, bufferSize)) == -1) {
        cout << "ERROR on getting the hostname." << endl;
        return -1;
    }

    strncpy(data.hostname, buffer, MAX_HOSTNAME_SIZE - 1);
    data.hostname[MAX_HOSTNAME_SIZE - 1] = '\0';

    memset(buffer, 0, sizeof(buffer));

    return 0;
}

int Station::getIpAddress(StationData &data)
{
    struct ifaddrs *netInterfaces, *tempInterface = NULL;

    if (!getifaddrs(&netInterfaces)) {
        tempInterface = netInterfaces;

        while (tempInterface != NULL) {
            if (tempInterface->ifa_addr->sa_family == AF_INET) {
                if (strcmp(tempInterface->ifa_name, "eth0") == 0) {
                    strncpy(data.ipAddress, inet_ntoa(((struct sockaddr_in *)tempInterface->ifa_addr)->sin_addr), IP_ADDRESS_SIZE - 1);
                    data.ipAddress[IP_ADDRESS_SIZE - 1] = '\0';
                }
            }

            tempInterface = tempInterface->ifa_next;
        }

        freeifaddrs(netInterfaces);
    } else {
        cout << "ERROR on getting IP Adress." << endl;
        return -1;
    }

    return 0;
}

int Station::getMacAddress(int sockfd, char *macAddress, size_t size) {
    struct ifreq ifr;

    strcpy(ifr.ifr_name, "eth0");

    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
        cerr << "ERROR on getting Mac Address." << endl;
        close(sockfd);
        return -1;
    }

    unsigned char *mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
    snprintf(macAddress, size, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return 0;
}

int Station::getStatus(Status &status)
{
    FILE *fp = popen("systemctl is-active systemd-timesyncd.service", "r");
    if (!fp) {
        std::cerr << "Failed to open power status file." << std::endl;
        return -1;
    }

    char result[10];
    if (fgets(result, sizeof(result), fp)) {
        pclose(fp);
        if (std::string(result).find("active") != std::string::npos) {
            status = Status::AWAKEN;
        } else {
            status = Status::ASLEEP;
        }

        return 0;
    } else {
        std::cerr << "Failed to read command output." << std::endl;
        pclose(fp);
        return -1;
    }
    
    return 0;
}


void Station::setSocketTimeout(int sockfd, int timeoutSec) {
    struct timeval timeout;
    timeout.tv_sec = timeoutSec;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
}


int Station::createSocket(int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        cerr << "ERROR opening socket." << endl;
        return -1;
    }

    setSocketTimeout(sockfd, 1); // Timeout de 1 segundo

    if (port != 0) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(addr.sin_zero), 8);

        if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0) {
            cerr << "ERROR on binding socket. (" << port << "): " << strerror(errno) << endl;
            close(sockfd);
            return -1;
        }
    }

    return sockfd;
}

void Station::setSocketBroadcastOptions(int sockfd) {
    const int optval{1};
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0) {
        throw std::runtime_error("Failed to set socket options");
    }
}

void Station::sendMessage(const char* destIP, Message msg, int port) {
    int sockfd = createSocket(0);
    // cout << "MANDANDO MENSNFSAJ,DHFSAJGHKSAGSD" << destIP << endl;
    struct sockaddr_in destAddr;
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);
    destAddr.sin_addr.s_addr = inet_addr(destIP);
    // inet_pton(AF_INET, destIP.c_str(), &destAddr.sin_addr);

    // cout << "MANDANDO MENSNFSAJ,DHFSAJGHKSAGSD" << destIP << endl;
    sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr*)&destAddr, sizeof(destAddr));
    close(sockfd);
}

Message Station::receiveMessage(int port) {
    int sockfd = createSocket(port);
    struct sockaddr_in senderAddr;
    socklen_t addrLen = sizeof(senderAddr);
    Message msg;

    recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr*)&senderAddr, &addrLen);
    close(sockfd);

    return msg;
}

void Station::startElection() {
    cout << "Station " << id << " is starting an election." << endl;

    bool higherExists = false;
    
    // Itera sobre a lista de StationData
    for (const StationData& station : discoveredClients) { 
        if (station.id > id) {  // Verifica se o id é maior
            // Cria uma mensagem com o tipo e PID
            Message electionMsg = {ELECTION, id};
            
            // Envia a mensagem para o endereço IP atual
            sendMessage(station.ipAddress, electionMsg, PORT_ELECTION);
        }
    }

    if (!higherExists) {
        type = Type::MANAGER;
        sendCoordinatorMessage();
    }
}


void Station::listenForCoordinator() {
    int sockfd = createSocket(PORT_COORDINATOR);
    setSocketTimeout(sockfd, 15);

    while (!stopThreads.load()) {

        struct sockaddr_in senderAddr;
        socklen_t addrLen = sizeof(senderAddr);
        Message msg;

        int recvStatus = recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr*)&senderAddr, &addrLen);

        if (recvStatus < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                std::cout << "Coordinator message timeout. Starting election." << std::endl;
                startElection();
                break;
            } else {
                std::cerr << "ERROR on receiving coordinator message: " << strerror(errno) << std::endl;
            }
        } else if (msg.type == COORDINATOR) {
            std::cout << "Coordinator message received from station " << msg.id << std::endl;
        }

        // Sleep for a short period before listening again (optional)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Close the socket only after exiting the loop
    close(sockfd);
}


void Station::sendCoordinatorMessage() {
    while (!stopThreads.load()) {
        Message coordMsg = {COORDINATOR, id};
        for (const StationData& station : discoveredClients) { 
            sendMessage(station.ipAddress, coordMsg, PORT_COORDINATOR);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
