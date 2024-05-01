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

#include "./discovery.h"

#define PORT 4000
#define BUFFER_SIZE 256

using namespace std;

namespace Client {
    int getClientHostname(char *buffer, size_t bufferSize, string &hostname) {
        memset(buffer, 0, sizeof(buffer));

        if ((gethostname(buffer, bufferSize)) == -1) {
            cout << "ERROR on getting the hostname." << endl;
            return -1;
        }

        hostname.assign(buffer);
        memset(buffer, 0, sizeof(buffer));

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

        /** Get hostname, MAC Address, IP Address, Status **/
        char buffer[BUFFER_SIZE];
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