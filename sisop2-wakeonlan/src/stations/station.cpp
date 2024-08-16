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
#include <thread>
#include <chrono>
#include <errno.h>
#include <time.h>

#include "./stations.hpp"

#define BUFFER_SIZE 256

using namespace std;

int Station::getHostname(char *buffer, size_t bufferSize, StationData &data)
{
    memset(buffer, 0, sizeof(buffer));

    if ((gethostname(buffer, bufferSize)) == -1)
    {
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

    if (!getifaddrs(&netInterfaces))
    {
        tempInterface = netInterfaces;

        while (tempInterface != NULL)
        {
            if (tempInterface->ifa_addr->sa_family == AF_INET)
            {
                if (strcmp(tempInterface->ifa_name, "eth0") == 0)
                {
                    strncpy(data.ipAddress, inet_ntoa(((struct sockaddr_in *)tempInterface->ifa_addr)->sin_addr), IP_ADDRESS_SIZE - 1);
                    data.ipAddress[IP_ADDRESS_SIZE - 1] = '\0';
                }
            }

            tempInterface = tempInterface->ifa_next;
        }

        freeifaddrs(netInterfaces);
    }
    else
    {
        cout << "ERROR on getting IP Adress." << endl;
        return -1;
    }

    return 0;
}

int Station::getMacAddress(int sockfd, char *macAddress, size_t size)
{
    struct ifreq ifr;

    strcpy(ifr.ifr_name, "eth0");

    if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
    {
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
    if (!fp)
    {
        std::cerr << "Failed to open power status file." << std::endl;
        return -1;
    }

    char result[10];
    if (fgets(result, sizeof(result), fp))
    {
        pclose(fp);
        if (std::string(result).find("active") != std::string::npos)
        {
            status = Status::AWAKEN;
        }
        else
        {
            status = Status::ASLEEP;
        }

        return 0;
    }
    else
    {
        std::cerr << "Failed to read command output." << std::endl;
        pclose(fp);
        return -1;
    }

    return 0;
}

void Station::setSocketTimeout(int sockfd, int timeoutSec)
{
    struct timeval timeout;
    timeout.tv_sec = timeoutSec;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
}

int Station::createSocket(int port)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        cerr << "ERROR opening socket." << endl;
        return -1;
    }

    // setSocketTimeout(sockfd, 1); // Timeout de 1 segundo

    if (port != 0)
    {
        int reuse = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
            std::cerr << "Failed to set SO_REUSEADDR: " << std::strerror(errno) << std::endl;
            close(sockfd);
            return -1;
        }
    
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        bzero(&(addr.sin_zero), 8);
        
        if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
        {
            cerr << "ERROR on binding socket. (" << port << "): " << strerror(errno) << endl;
            close(sockfd);
            return -1;
        }
    }

    return sockfd;
}

void Station::setSocketBroadcastOptions(int sockfd)
{
    const int optval{1};
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0)
    {
        throw std::runtime_error("Failed to set socket options");
    }
}


void Station::setSocketReuseOptions(int sockfd)
{
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "Failed to set SO_REUSEADDR: " << strerror(errno) << endl;
    }
}


int Station::getLastFieldOfIP(const char* ip) {
    // Procura a última ocorrência de '.' no IP
    const char* lastDot = strrchr(ip, '.');
    
    // Verifica se encontrou um ponto
    if (lastDot != nullptr) {
        // Converte a parte após o último ponto para um inteiro
        return atoi(lastDot + 1);
    }

    // Retorna -1 se o IP não for válido
    return -1;
}

void Station::startElection() {
    bool receivedOk = false;
    int sockfd = createSocket(PORT_ELECTION);  // Abra o socket uma vez
    setSocketTimeout(sockfd, 1);
    bool a = false;
    for (StationData& client : discoveredClients) {
        int clientId = getLastFieldOfIP(client.ipAddress);
        cout << client.hostname << endl;
        cout << clientId << endl;
        cout << id << endl;
        if (clientId > id) {
            cout << "enviando msg" << endl;
            // Envia mensagem de eleição
            Message electionMessage = {MessageType::ELECTION, id};
            // Enviar mensagem de eleição ao cliente
            sendMessage(sockfd, client, electionMessage);
            a = true;
        }
    }


    // Aguardar resposta do cliente
    receivedOk = waitForOkMessage();
    // Se não recebeu nenhuma mensagem de OK
    if (!receivedOk && !a) {
        // Autoproclama-se líder
        type = Type::MANAGER;
        cout << "Eu sou o novo coordenador (ID: " << id << ")" << endl;
        // Envia mensagem de coordenação para todos
        sendCoordinatorMessage();
    }

    close(sockfd); // Fecha o socket após o loop
}

void Station::sendMessage(int sockfd, const StationData& client, const Message& message) {
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_ELECTION);  // Use a mesma porta para coordenação
    inet_pton(AF_INET, client.ipAddress, &addr.sin_addr);

    sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr*)&addr, sizeof(addr));
}

bool Station::waitForOkMessage() {
    int sockfd = createSocket(PORT_ELECTION_RESPONSE);
    setSocketTimeout(sockfd, 10);
    struct sockaddr_in senderAddr;
    socklen_t addrLen = sizeof(senderAddr);
    char buffer[BUFFER_SIZE];

    // setSocketTimeout(sockfd, 3); // Timeout de 2 segundos
    while (!stopThreads.load()){
        int receivedBytes = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&senderAddr, &addrLen);
        if (receivedBytes > 0) {
            Message* message = (Message*)buffer;
            if (message->type == MessageType::OK) {
                char clientIP[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &senderAddr.sin_addr, clientIP, sizeof(clientIP));
                std::cout << "Solicitação recebida de: " << clientIP << std::endl;
                close(sockfd);
                return true;
                break;
            }
        } else if (errno == EWOULDBLOCK || errno == EAGAIN) {
            perror ("Erro no waitForOkMessage");
            close(sockfd);
            return false;  // Continue trying to receive data
            break;
        }
    }
    close(sockfd);
    return false;
}

void Station::sendCoordinatorMessage() {
    int sockfd = createSocket(PORT_COORDINATOR);  // Abra o socket uma vez
    setSocketTimeout(sockfd, 1);
    
    while (!stopThreads.load()) {
        for (const StationData& client : discoveredClients) {  // Use const StationData
            // Envia mensagem de coordenação
            Message coordinatorMessage = {MessageType::COORDINATOR, id};
            sendMessage(sockfd, client, coordinatorMessage);  // Chame a função com const
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));  // Evita loop rápido contínuo
    }

    close(sockfd);  // Fecha o socket após o loop
}

void Station::listenForElectionMessages() {
    Station station;
    int recvSockfd = station.createSocket(PORT_ELECTION);
    if (recvSockfd < 0) {
        cerr << "Failed to create socket for listening to election messages." << endl;
        return;
    }
    station.setSocketTimeout(recvSockfd, 1);

    struct sockaddr_in senderAddr;
    socklen_t addrLen = sizeof(senderAddr);
    Message receivedMessage;

    while (!stopThreads.load()) {
        int bytesReceived = recvfrom(recvSockfd, &receivedMessage, sizeof(receivedMessage), 0, (struct sockaddr *)&senderAddr, &addrLen);
        if (bytesReceived > 0) {
            cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << endl;
            cout << "Received election message" << endl;
            cout << "ID: " << id << endl;
            cout << "Message type: " << receivedMessage.type << endl;
            cout << "Message ID: " << receivedMessage.id << endl;
            if (receivedMessage.type == MessageType::ELECTION && receivedMessage.id < id) {
                // Responde com a mensagem "OK" se seu ID for maior
                station.sendOkResponse(senderAddr);
            }
        }else{
            perror("Erro ao listenForElectionMessages");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    close(recvSockfd);
}

void Station::sendOkResponse(const sockaddr_in& senderAddr) {
    int sendSockfd = createSocket(PORT_ELECTION_RESPONSE);
    if (sendSockfd < 0) {
        cerr << "Failed to create socket for sending OK response." << endl;
        return;
    }

    // Envia a mensagem "OK"
    Message okMessage = {MessageType::OK, id};
    sendto(sendSockfd, &okMessage, sizeof(okMessage), 0, (struct sockaddr *)&senderAddr, sizeof(senderAddr));

    close(sendSockfd);
}
