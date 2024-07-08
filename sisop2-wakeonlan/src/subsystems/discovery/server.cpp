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

#include "discovery.hpp"

#define BUFFER_SIZE 256

using namespace std;

// Variáveis globais
std::vector<DiscoveredData> discoveredClients; // Lista de dados descobertos
std::mutex mtx; // Mutex para sincronização de acesso à lista

int Server::requestSleepStatus(const char *ipAddress, RequestData request, Status &status) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        cerr << "ERROR opening socket." << endl;
        return -1;
    }

    struct sockaddr_in recipient_addr;
    memset(&recipient_addr, 0, sizeof(recipient_addr));
    recipient_addr.sin_family = AF_INET;
    recipient_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, ipAddress, &recipient_addr.sin_addr) <= 0) {
        cerr << "ERROR invalid address/ Address not supported." << endl;
        close(sockfd);
        return -1;
    }

    if (sendto(sockfd, &request, sizeof(request), 0, (struct sockaddr *) &recipient_addr, sizeof(recipient_addr)) < 0) {
        cerr << "ERROR sending request." << endl;
        close(sockfd);
        return -1;
    }

    cout << "mandei msg para: " << ipAddress << endl;

    // Receber resposta
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    Status responseStatus;
    ssize_t bytesReceived = recvfrom(sockfd, &responseStatus, sizeof(responseStatus), 0, (struct sockaddr *) &from, &fromlen);
    if (bytesReceived < 0) {
        cerr << "ERROR receiving response." << endl;
        close(sockfd);
        return -1;
    }

    status = responseStatus;
    close(sockfd);
    return 0;

    close(sockfd);
    return 0;
}

int Server::sendSocket() {
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        cerr << "ERROR opening socket." << endl;

    struct sockaddr_in participant_addr;
    participant_addr.sin_family = AF_INET;
    participant_addr.sin_port = htons(PORT_S);
    participant_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(participant_addr.sin_zero), 8);

    if (bind(sockfd, (struct sockaddr *) &participant_addr, sizeof(struct sockaddr)) < 0)
        cerr << "ERROR on binding socket." << endl;

    sockaddr_in cli_addr;
    socklen_t clilen = sizeof(struct sockaddr_in);

    while (true) {
        // Receive from a socket
        DiscoveredData receivedData;
        memset(&receivedData, 0, sizeof(receivedData));

        ssize_t bytesReceived = recvfrom(sockfd, &receivedData, sizeof(receivedData), 0, (struct sockaddr *) &cli_addr, &clilen);
        if (bytesReceived < 0) {
            cerr << "ERROR on recvfrom." << endl;
            break;
        }

        // Lock mutex before accessing the shared list
        {
            std::lock_guard<std::mutex> lock(mtx);
            discoveredClients.push_back(receivedData);
        }

        cout << "Hostname: " << receivedData.hostname << endl;
        cout << "IP Address: " << receivedData.ipAddress << endl;
        cout << "MAC Address: " << receivedData.macAddress << endl;
        cout << "Status: " << receivedData.status << endl;

        // Send socket
        if (sendto(sockfd, "Got your message\n", 17, 0, (struct sockaddr *) &cli_addr, sizeof(struct sockaddr)) < 0)
            cerr << "ERROR on sendto." << endl;
    }

    close(sockfd);
    return 0;
}

std::vector<DiscoveredData> Server::getDiscoveredClients() {
    std::lock_guard<std::mutex> lock(mtx);
    return discoveredClients;
}
