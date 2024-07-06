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

#include "./discovery.hpp"

#define BUFFER_SIZE 256

using namespace std;

int Client::getHostname(char *buffer, size_t bufferSize, string &hostname) {
    memset(buffer, 0, sizeof(buffer));

    if ((gethostname(buffer, bufferSize)) == -1) {
        cout << "ERROR on getting the hostname." << endl;
        return -1;
    }

    hostname.assign(buffer);
    memset(buffer, 0, sizeof(buffer));

    return 0;
}

int Client::getIpAddress(string &ipAddress) {
    struct ifaddrs *netInterfaces, *tempInterface = NULL;

    if (!getifaddrs(&netInterfaces)) {
        tempInterface = netInterfaces;

        while(tempInterface != NULL) {
            if(tempInterface->ifa_addr->sa_family == AF_INET) {
                if(strcmp(tempInterface->ifa_name, "eth0") == 0){
                    ipAddress=inet_ntoa(((struct sockaddr_in*)tempInterface->ifa_addr)->sin_addr);
                }
            }

            tempInterface = tempInterface->ifa_next;
        }
    } else {
        cout << "ERROR on getting IP Adress." << endl;
        return -1;
    }

    return 0;
}

int Client::getMacAddress(int sockfd, char *macAddress, size_t size) {
    struct ifreq ifr;

    strcpy(ifr.ifr_name, "eth0");

    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0) {
        cerr << "ERROR on getting Mac Address." << endl;
        close(sockfd);
        return -1;
    }

    unsigned char* mac = (unsigned char*)ifr.ifr_hwaddr.sa_data;
    snprintf(macAddress, size, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return 0;
}

// TODO: Check if it's correct - probably not
int Client::getStatus(Status &status) {
    FILE* fp = popen("systemctl is-active systemd-timesyncd.service", "r");
    // FILE* fp = popen("service systemd-timesyncd status", "r");
    if (!fp) {
        std::cerr << "Failed to open power status file." << std::endl;
        return -1;
    }

    char result[10];
    if (fgets(result, sizeof(result), fp)) {
        pclose(fp);
        // Check if the service is active
        if (std::string(result).find("active") != std::string::npos) {
        // if (std::string(result).find("Active: active (running)") != std::string::npos) {
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
}

    // if (power.is_open()) {
    //     int returnStatus;
    //     power >> returnStatus;

    //     status = intToStatus(returnStatus);
    //     power.close();

    //     return 0;
    // } else {
    //     cerr << "ERROR on checking PC status." << endl;
    //     return -1;
    // }

int Client::sendSocket(int argc, const char *serverHostname) {
    // serverHostname not provided
    if (argc < 2) {
        cerr << "Usage " << serverHostname << " hostname." << endl;
        exit(0);
    }
    
    struct hostent *server;
    server = gethostbyname(serverHostname);
    if (server == NULL) {
        cerr << "ERROR no such host." << endl;
        exit(0);
    }
    
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        cerr << "ERROR opening socket." << endl;

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;     
    serv_addr.sin_port = htons(SERVER_PORT);    
    serv_addr.sin_addr = *((struct in_addr *) server -> h_addr);
    bzero(&(serv_addr.sin_zero), 8);  

    char buffer[BUFFER_SIZE];

    DiscoveredData pcData;
    getHostname(buffer, BUFFER_SIZE, pcData.hostname);
    getIpAddress(pcData.ipAddress);
    getMacAddress(sockfd, pcData.macAddress, MAC_ADDRESS_SIZE);
    getStatus(pcData.status);

    cout << "Hostname: " << pcData.hostname << endl;
    cout << "IP Address: " << pcData.ipAddress << endl;
    cout << "Mac Address: " << pcData.macAddress << endl;
    cout << "Status: " << pcData.status << endl;

    /** Uncomment if necessary **/
    // printf("Enter the message: ");
    // bzero(buffer, 256);
    // fgets(buffer, 256, stdin);

    if (sendto(sockfd, &pcData, sizeof(pcData), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in)) < 0)
        cerr << "ERROR on sendto." << endl;
    
    struct sockaddr_in from;
    unsigned int length = sizeof(struct sockaddr_in);
    if (recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) &from, &length) < 0)
        cerr << "ERROR on recvfrom." << endl;

    cout << "Got an ack: " << buffer << endl;
    
    close(sockfd);
    return 0;
}