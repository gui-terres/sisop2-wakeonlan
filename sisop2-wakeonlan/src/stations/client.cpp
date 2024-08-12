#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fstream>
#include <chrono>  // Para std::this_thread::sleep_for

#include "./stations.hpp"

#define BUFFER_SIZE 256
#define BUFFER_SIZE2 1024

using namespace std;

int Client::enterWakeOnLan(int argc) {
    int sockfd = createSocket();
    if (sockfd == -1) return 1;

    setSocketBroadcastOptions(sockfd);

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_SOCKET);
    serv_addr.sin_addr.s_addr = inet_addr(BROADCAST_ADDR);
    bzero(&(serv_addr.sin_zero), 8);

    char buffer[BUFFER_SIZE];

    StationData pcData;
    getHostname(buffer, BUFFER_SIZE, pcData);
    getIpAddress(pcData);
    getMacAddress(sockfd, pcData.macAddress, MAC_ADDRESS_SIZE);
    pcData.status = Status::AWAKEN;

    if (sendto(sockfd, &pcData, sizeof(pcData), 0, (const struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0)
        cerr << "ERROR on sendto." << endl;

    close(sockfd);
    return 0;
}

void Client::waitForSleepRequests() {
    int sockfd = createSocket(PORT_SLEEP);
    if (sockfd == -1) return;

    while (true) {
        RequestData request;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        ssize_t bytesReceived = recvfrom(sockfd, &request, sizeof(request), 0, (struct sockaddr *)&from, &fromlen);
        if (bytesReceived < 0) {
            cerr << "ERROR on recvfrom." << endl;
            continue;
        }

        if (request.request == Request::SLEEP_STATUS) {
            Status status = Status::AWAKEN;
            // printf("oiii\n");
            // cout << inet_ntoa(from.sin_addr) << endl;
            sendto(sockfd, &status, sizeof(status), 0, (struct sockaddr *)&from, fromlen);
        }
    }

    close(sockfd);
}

int Client::getManagerData() {
    int sockfd = createSocket(PORT_MANAGER_DATA);
    if (sockfd == -1) return -2;

    struct timeval tv;
    tv.tv_sec = 5;  // Tempo de espera em segundos
    tv.tv_usec = 0; // Tempo de espera em microsegundos
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
        cerr << "ERROR setting socket timeout." << endl;
        close(sockfd);
        return -2;
    }

    sockaddr_in cli_addr;
    socklen_t clilen = sizeof(struct sockaddr_in);

    memset(&managerInfo, 0, sizeof(managerInfo));

    while (true) {
        ssize_t bytesReceived = recvfrom(sockfd, &managerInfo, sizeof(managerInfo), 0, (struct sockaddr *)&cli_addr, &clilen);
        if (bytesReceived < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                cerr << "ERROR: Timeout receiving response. Retrying..." << endl;
                continue;  // Continue trying to receive data
            } else {
                cerr << "ERROR receiving response: " << strerror(errno) << endl;
                close(sockfd);
                return -2;
            }
        } else {
            // Data received successfully
            break;
        }
    }

    cout << "Manager Info" << endl;
    cout << "Hostname: " << managerInfo.hostname << endl;
    cout << "IP Address: " << managerInfo.ipAddress << endl;
    cout << "Mac Address: " << managerInfo.macAddress << endl;

    close(sockfd);
    return 0;
}

int Client::sendExitRequest(const char *ipAddress) {
    int sockfd = createSocket(PORT_EXIT);
    if (sockfd == -1) return -1;

    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
        cerr << "ERROR setting socket timeout." << endl;
        close(sockfd);
        return -1;
    }

    struct sockaddr_in recipient_addr;
    memset(&recipient_addr, 0, sizeof(recipient_addr));
    recipient_addr.sin_family = AF_INET;
    recipient_addr.sin_port = htons(PORT_EXIT);
    if (inet_pton(AF_INET, ipAddress, &recipient_addr.sin_addr) <= 0) {
        cerr << "ERROR invalid address/ Address not supported." << endl;
        close(sockfd);
        return -1;
    }

    RequestData req;
    req.request = Request::EXIT;

    if (sendto(sockfd, &req, sizeof(req), 0, (struct sockaddr *)&recipient_addr, sizeof(recipient_addr)) < 0) {
        cerr << "ERROR sending request." << endl;
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}
/*
void Client::sendMessage(const std::string &message, const std::string &ipAddress) {
    int sockfd;
    struct sockaddr_in serv_addr;

    // Criando o socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "ERROR creating socket" << endl;
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_TABLE); // Use a porta apropriada para comunicação com o gerenciador

    // Convertendo endereço IP para formato binário
    if (inet_pton(AF_INET, ipAddress.c_str(), &serv_addr.sin_addr) <= 0) {
        cerr << "ERROR: Invalid address / Address not supported" << endl;
        close(sockfd);
        return;
    }

    // Enviando a mensagem "oi"
    if (sendto(sockfd, message.c_str(), message.size(), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "ERROR sending message" << endl;
        close(sockfd);
        return;
    }

    //cout << "Message sent: " << message << endl;
    cout << "Eu quero a lista de participantes!!!" << endl;

    // Fechando o socket
    close(sockfd);
}
*/

void Client::askForTable() {
    int sockfd;
    struct sockaddr_in servaddr;

    // Criar socket UDP
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Erro ao criar socket");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Configurar informações do servidor
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT_TABLE);
    //servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_addr.s_addr = inet_addr("172.18.0.15"); // IP do manager
    socklen_t len = sizeof(servaddr);

    while (true) {
         std::this_thread::sleep_for(std::chrono::seconds(4));
        // Enviar solicitação ao manager
        const char *message = "Solicitar tabela";
        sendto(sockfd, message, strlen(message), MSG_CONFIRM, (const struct sockaddr *)&servaddr, len);

        std::cout << "Solicitação enviada" << std::endl;

        // Receber a resposta do manager
        std::vector<StationData> receivedData(10);  // Supondo um tamanho inicial
        int n = recvfrom(sockfd, receivedData.data(), receivedData.size() * sizeof(StationData), MSG_WAITALL, (struct sockaddr *)&servaddr, &len);

        // Ajustar o tamanho do vetor conforme o número de elementos recebidos
        receivedData.resize(n / sizeof(StationData));

        // Processar a resposta recebida
        std::cout << "Informações recebidas do manager: " << std::endl;
        for (const auto& data : receivedData) {
            std::cout << "Hostname: " << data.hostname << ", IP: " << data.ipAddress << ", MAC: " << data.macAddress << ", Status: " << static_cast<int>(data.status) << std::endl;
        }
    }

    close(sockfd);
}