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

int Station::createSocket(int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        cerr << "ERROR opening socket." << endl;
        return -1;
    }

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

// void Station::waitForParticipantDataRequests() {
//     int sockfd = createSocket(PORT_DATA);
//     if (sockfd == -1) return;

//     while (true) {
//         RequestData request;
//         struct sockaddr_in from;
//         socklen_t fromlen = sizeof(from);
//         ssize_t bytesReceived = receiveData(sockfd, &request, sizeof(request), &from, &fromlen);
//         if (bytesReceived < 0) {
//             cerr << "ERROR on recvfrom." << endl;
//             continue;
//         }

//         if (request.request == Request::PARTICIPANT_DATA) {
//             char buffer[BUFFER_SIZE];

//             StationData pcData;
//             getHostname(buffer, BUFFER_SIZE, pcData);
//             getIpAddress(pcData);
//             getMacAddress(sockfd, pcData.macAddress, MAC_ADDRESS_SIZE);
//             pcData.status = Status::AWAKEN;

//             sendData(sockfd, &pcData, sizeof(pcData), inet_ntoa(from.sin_addr), PORT_DATA);
//         }
//     }

//     close(sockfd);
// }



