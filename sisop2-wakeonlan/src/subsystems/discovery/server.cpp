#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./discovery.hpp"

using namespace std;

int Server::sendSocket() {
    int sockfd;
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        cerr << "ERROR opening socket." << endl;

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0) 
        cerr << "ERROR on binding socket." << endl;
    
    // char buffer[256];

    sockaddr_in cli_addr;
    socklen_t clilen = sizeof(struct sockaddr_in);

    while (true) {
        // Receive from a socket
        DiscoveredData receivedData;
        if (recvfrom(sockfd, &receivedData, sizeof(receivedData), 0, (struct sockaddr *) &cli_addr, &clilen)) 
            cerr << "ERROR on recvfrom." << endl;
        
        // cout << "Received a datagram: " << buffer << endl;   MUST BE DELETED
        
        // Send socket
        if (sendto(sockfd, "Got your message\n", 17, 0,(struct sockaddr *) &cli_addr, sizeof(struct sockaddr)) < 0) 
            cerr << "ERROR on sendto." << endl;
    }
    
    close(sockfd);
    return 0;
}