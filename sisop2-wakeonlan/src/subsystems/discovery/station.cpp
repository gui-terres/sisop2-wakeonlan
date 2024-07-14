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

int Station::getHostname(char *buffer, size_t bufferSize, DiscoveredData &data)
{
    memset(buffer, 0, sizeof(buffer));

    if ((gethostname(buffer, bufferSize)) == -1)
    {
        cout << "ERROR on getting the hostname." << endl;
        return -1;
    }

    strncpy(data.hostname, buffer, MAX_HOSTNAME_SIZE - 1);
    data.hostname[MAX_HOSTNAME_SIZE - 1] = '/0';

    memset(buffer, 0, sizeof(buffer));

    return 0;
}

int Station::getIpAddress(DiscoveredData &data)
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
    // FILE* fp = popen("service systemd-timesyncd status", "r");
    if (!fp)
    {
        std::cerr << "Failed to open power status file." << std::endl;
        return -1;
    }

    char result[10];
    if (fgets(result, sizeof(result), fp))
    {
        pclose(fp);
        // Check if the service is active
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

    // não fazendo nada, só ficando com o valor original
    return 0;
}