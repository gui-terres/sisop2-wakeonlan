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

#include "./discovery.h"

#define PORT 4000
#define BUFFER_SIZE 256

using namespace std;

namespace Client {
    int getHostname(char *buffer, size_t bufferSize, string &hostname) {
        memset(buffer, 0, sizeof(buffer));

        if ((gethostname(buffer, bufferSize)) == -1) {
            cout << "ERROR on getting the hostname." << endl;
            return -1;
        }

        hostname.assign(buffer);
        memset(buffer, 0, sizeof(buffer));

        return 0;
    }

    int getIpAddress(string &ipAddress) {
        struct ifaddrs *netInterfaces, *tempInterface = NULL;

        if (!getifaddrs(&netInterfaces)) {
            tempInterface = netInterfaces;

            while(tempInterface != NULL) {
                if(tempInterface->ifa_addr->sa_family == AF_INET) {
                    if(strcmp(tempInterface->ifa_name, "eth0")){
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

    int getMacAddress(int sockfd, char *macAddress, size_t size) {
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

    int socket(int argc, const char *serverHostname) {
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
        serv_addr.sin_port = htons(PORT);    
        serv_addr.sin_addr = *((struct in_addr *) server -> h_addr);
        bzero(&(serv_addr.sin_zero), 8);  
   
        char buffer[BUFFER_SIZE];

        discoveredData data;
        getHostname(buffer, BUFFER_SIZE, data.hostname);
        getIpAddress(data.ipAddress);
        getMacAddress(sockfd, data.macAddress, MAC_ADDRESS_SIZE);

        cout << "Hostname: " << data.hostname << endl;
        cout << "IP Address: " << data.ipAddress << endl;
        cout << "Mac Address: " << data.macAddress << endl;

        printf("Enter the message: ");
        bzero(buffer, 256);
        fgets(buffer, 256, stdin);

        if (sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in)) < 0)
            cerr << "ERROR on sendto." << endl;
        
        struct sockaddr_in from;
        unsigned int length = sizeof(struct sockaddr_in);
        if (recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) &from, &length) < 0)
            cerr << "ERROR on recvfrom." << endl;

        cout << "Got an ack: " << buffer << endl;
        
        close(sockfd);
        return 0;
    }
}