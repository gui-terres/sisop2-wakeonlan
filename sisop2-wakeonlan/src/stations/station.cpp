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
#include <ctime>

#include "./stations.hpp"

#define BUFFER_SIZE 256

using namespace std;

int Station::getHostname(char *buffer, size_t bufferSize, StationData &data)
{
    memset(buffer, 0, sizeof(buffer));

    if ((gethostname(buffer, bufferSize)) == -1)
    {
        // cout << "ERROR on getting the hostname." << endl;
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
        // cout << "ERROR on getting IP Adress." << endl;
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
        // cerr << "ERROR on getting Mac Address." << endl;
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
        // std:: cerr << "Failed to open power status file." << std::endl;
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
        // std:: cerr << "Failed to read command output." << std::endl;
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
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        {
            std:: cerr << "Failed to set SO_REUSEADDR: " << std::strerror(errno) << std::endl;
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
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        // cerr << "Failed to set SO_REUSEADDR: " << strerror(errno) << endl;
    }
}

int Station::getLastFieldOfIP(const char *ip)
{
    // Procura a última ocorrência de '.' no IP
    const char *lastDot = strrchr(ip, '.');

    // Verifica se encontrou um ponto
    if (lastDot != nullptr)
    {
        // Converte a parte após o último ponto para um inteiro
        return atoi(lastDot + 1);
    }

    // Retorna -1 se o IP não for válido
    return -1;
}

void tempo(){
    std::time_t now = std::time(nullptr);
    // Converte o tempo para uma string legível
    std::tm* localTime = std::localtime(&now);
    // Imprime a data e hora atual
    std::cout << "Data e hora atual: " << std::asctime(localTime);
}

void Station::startElection()
{
    bool receivedOk = false;
    Client client;
    RequestData req;
    Status status;
    int result = client.requestSleepStatus(managerInfo.ipAddress, req, status);
    tempo();
    int msgCount = 0;
    cout << "AAAAAAAAAAAAAAAAA " << result << endl;
    // std::cout << "Request result for " << client.ipAddress << ": " << result << std::endl;

    if (result != 0)
    {
        for (StationData &client : discoveredClients)
        {   
            tempo();
            int clientId = getLastFieldOfIP(client.ipAddress);
            cout << client.hostname << endl;
            cout << clientId << endl;
            cout << id << endl;
            if (clientId > id)
            {   
                cout << "enviando msg" << endl;
                // Envia mensagem de eleição
                int sockfd = createSocket(PORT_ELECTION); // Abra o socket uma vez
                setSocketTimeout(sockfd, 1);
                Message electionMessage = {MessageType::ELECTION, id};
                // Enviar mensagem de eleição ao cliente
                struct sockaddr_in recipient_addr;
                memset(&recipient_addr, 0, sizeof(recipient_addr));
                recipient_addr.sin_family = AF_INET;
                recipient_addr.sin_port = htons(PORT_ELECTION);
                if (inet_pton(AF_INET, client.ipAddress, &recipient_addr.sin_addr) <= 0)
                {
                    cerr << "ERROR invalid address/ Address not supported." << endl;
                    close(sockfd);
                    return;
                }

                cout << "mandando eleição" << endl;
                tempo();
                if (sendto(sockfd, &electionMessage, sizeof(electionMessage), 0, (struct sockaddr *)&recipient_addr, sizeof(recipient_addr)) < 0)
                {
                    perror("ERROR sending election message");
                    return;
                }
                msgCount+=1;
                close(sockfd);
            }
        }

        // Aguardar resposta do cliente
        // receivedOk = waitForOkMessage();
        // Se não recebeu nenhuma mensagem de OK

        int sockfd = createSocket(PORT_ELECTION_RESPONSE);
        setSocketTimeout(sockfd, 10);
        while (!stopThreads.load() && msgCount > 0){
            cout << "esperando pela resposta" << endl;
            tempo();
            receivedOk = waitForOkMessage(sockfd);  
            tempo();
            cout << "tenho minha resposta" << receivedOk << endl;
            if (receivedOk == true){
                break;
            }
            msgCount-=1;
        }
        close(sockfd);

        if (!receivedOk)
        {
            // Autoproclama-se líder
            cout << "Eu sou o novo coordenador (ID: " << id << ")" << endl;
            type = Type::MANAGER;
            // Envia mensagem de coordenação para todos
            sendCoordinatorMessage();
        }
    }
}

void Station::sendMessage(int port, int sockfd, const StationData &client, const Message &message)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port); // Use a mesma porta para coordenação
    inet_pton(AF_INET, client.ipAddress, &addr.sin_addr);

    cout << "TAMANHO: " << sizeof(message)  << " para " << client.ipAddress << endl;
    cout << "TAMANHO: " << sizeof(message)  << " para " << client.ipAddress << endl;
    cout << "TAMANHO: " << sizeof(message)  << " para " << client.ipAddress << endl;
    sendto(sockfd, &message, sizeof(message), 0, (struct sockaddr *)&addr, sizeof(addr));
}

bool Station::waitForOkMessage(int sockfd)
{
    // setSocketTimeout(sockfd, 3); // Timeout de 2 segundos
    while (!stopThreads.load())
    {
        Message msg;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        ssize_t receivedBytes = recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&from, &fromlen);

        if (receivedBytes > 0)
        {
            if (msg.type == MessageType::OK || msg.type == MessageType::COORDINATOR)
            {
                char clientIP[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &from.sin_addr, clientIP, sizeof(clientIP));
                tempo();
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                std::cout << "Resposta recebida de: " << clientIP << std::endl;
                close(sockfd);
                return true;
                break;
            } 
        }
        else
        {
            perror("Erro no waitForOkMessage");
            perror("Erro no waitForOkMessage");
            perror("Erro no waitForOkMessage");
            perror("Erro no waitForOkMessage");
            perror("Erro no waitForOkMessage");
            perror("Erro no waitForOkMessage");
            perror("Erro no waitForOkMessage");
            perror("Erro no waitForOkMessage");
            perror("Erro no waitForOkMessage");
            close(sockfd);
            return false; // Continue trying to receive data
            break;
        }
    }
    close(sockfd);
    return false;
}

void Station::sendCoordinatorMessage()
{
    int sockfd = createSocket(PORT_ELECTION_RESPONSE); // Abra o socket uma vez
    setSocketTimeout(sockfd, 1);

    // while (!stopThreads.load())
    // {
    for (const StationData &client : discoveredClients)
    { // Use const StationData
        // Envia mensagem de coordenação
        cout << "Mandando quem é o líder nessa porra (" << client.ipAddress << ")" << endl;
        Message coordinatorMessage = {MessageType::COORDINATOR, id};
        if(strcmp(managerInfo.ipAddress, client.ipAddress)){
            tempo();
            sendMessage(PORT_ELECTION_RESPONSE, sockfd, client, coordinatorMessage); // Chame a função com const
        } else {
            cout << "hihiih sou eu ><" << endl;
        }
        // std::this_thread::sleep_for(std::chrono::seconds(1)); // Evita loop rápido contínuo
    }
    // }

    close(sockfd); // Fecha o socket após o loop
}

void Station::listenForElectionMessages()
{
    int sockfd = createSocket(PORT_ELECTION);
    if (sockfd == -1)
        return;
    setSocketTimeout(sockfd, 5);

    while (!stopThreads.load())
    {
        // Adicione um log ou printf para verificar o status de stopThreads
        // std::cout << "stopThreads: " << stopThreads.load() << std::endl;

        Message msg;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        ssize_t bytesReceived = recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&from, &fromlen);

        if (bytesReceived < 0)
        {
            perror("ERROR on recvfrom in listenForElectionMessages.");
            continue;
        }

        if (msg.type == MessageType::ELECTION)
        {   
            tempo();
            cout << "gremiooooooooooooooooo" << endl;
            cout << "gremiooooooooooooooooo" << endl;
            cout << "gremiooooooooooooooooo" << endl;
            cout << "gremiooooooooooooooooo" << endl;
            cout << "gremiooooooooooooooooo" << endl;
            cout << "gremiooooooooooooooooo" << endl;

            int responseSockfd = createSocket(PORT_ELECTION_RESPONSE); // Abra o socket uma vez
            setSocketTimeout(responseSockfd, 1);
            Message answerMessage = {MessageType::OK, id};
            // Enviar mensagem de eleição ao cliente
            struct sockaddr_in recipient_addr;
            memset(&recipient_addr, 0, sizeof(recipient_addr));
            recipient_addr.sin_family = AF_INET;
            recipient_addr.sin_port = htons(PORT_ELECTION_RESPONSE);
            recipient_addr.sin_addr = from.sin_addr; 

            this_thread::sleep_for(chrono::milliseconds(200));
            tempo();
            if (sendto(responseSockfd, &answerMessage, sizeof(answerMessage), 0, (struct sockaddr *)&recipient_addr, sizeof(recipient_addr)) < 0)
            {
                perror("ERROR sending election message");
                return;
            }
            close(responseSockfd);
        }
    }

    close(sockfd);
}


void Station::sendOkResponse(const sockaddr_in &senderAddr)
{
    int sendSockfd = createSocket(PORT_ELECTION_RESPONSE);
    if (sendSockfd < 0)
    {
        // cerr << "Failed to create socket for sending OK response." << endl;
        return;
    }

    // Envia a mensagem "OK"
    Message okMessage = {MessageType::OK, id};
    sendto(sendSockfd, &okMessage, sizeof(okMessage), 0, (struct sockaddr *)&senderAddr, sizeof(senderAddr));

    close(sendSockfd);
}

void Station::waitForSleepRequests()
{
    int sockfd = createSocket(PORT_SLEEP);
    if (sockfd == -1)
        return;
    setSocketTimeout(sockfd, 5);

    while (!stopThreads.load())
    {
        // Adicione um log ou printf para verificar o status de stopThreads
        // std::cout << "stopThreads: " << stopThreads.load() << std::endl;

        RequestData request;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        ssize_t bytesReceived = recvfrom(sockfd, &request, sizeof(request), 0, (struct sockaddr *)&from, &fromlen);

        if (bytesReceived < 0)
        {
            perror("ERROR on recvfrom in waitForSleepRequests.");
            continue;
        }

        if (request.request == Request::SLEEP_STATUS)
        {
            Status status = Status::AWAKEN;
            sendto(sockfd, &status, sizeof(status), 0, (struct sockaddr *)&from, fromlen);
        }
    }

    close(sockfd);
}

int Station::requestSleepStatus(const char *ipAddress, RequestData request, Status &status)
{
    int sockfd = createSocket(PORT_SLEEP);
    cout << ipAddress << endl;
    cout << ipAddress << endl;
    cout << ipAddress << endl;
    cout << ipAddress << endl;
    cout << ipAddress << endl;
    cout << ipAddress << endl;
    cout << ipAddress << endl;
    cout << ipAddress << endl;

    // Definir tempo limite de 1 segundo para recebimento
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv)) < 0)
    {
        cerr << "ERROR setting socket timeout." << endl;
        close(sockfd);
        return -1;
    }

    struct sockaddr_in recipient_addr;
    memset(&recipient_addr, 0, sizeof(recipient_addr));
    recipient_addr.sin_family = AF_INET;
    recipient_addr.sin_port = htons(PORT_SLEEP);
    if (inet_pton(AF_INET, ipAddress, &recipient_addr.sin_addr) <= 0)
    {
        cerr << "ERROR invalid address/ Address not supported." << endl;
        close(sockfd);
        return -1;
    }

aqui:
    if (sendto(sockfd, &request, sizeof(request), 0, (struct sockaddr *)&recipient_addr, sizeof(recipient_addr)) < 0)
    {
        perror("ERROR sending request requestSleepStatus");
        if (errno == ENETUNREACH)
        {
            goto aqui;
        }
        close(sockfd);
        return -1;
    }

    // Receber resposta
    struct sockaddr_in from;
    socklen_t fromlen = sizeof(from);
    Status responseStatus = Status::AWAKEN;

    ssize_t bytesReceived = recvfrom(sockfd, &responseStatus, sizeof(responseStatus), 0, (struct sockaddr *)&from, &fromlen);
    if (bytesReceived < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            cerr << "ERROR: Timeout receiving response in requestSleepStatus." << endl;
            if (type == Type::PARTICIPANT)
            {
                close(sockfd);
                return -1;
            }
        }
        else
        {
            perror("ERROR receiving response.");
        }
        status = Status::ASLEEP;
        close(sockfd);
        return 0;
    }

    status = responseStatus;
    close(sockfd);
    return 0;
}

void Station::tempo(){
    std::time_t now = std::time(nullptr);
    // Converte o tempo para uma string legível
    std::tm* localTime = std::localtime(&now);
    // Imprime a data e hora atual
    std::cout << "Data e hora atual: " << std::asctime(localTime);
}
