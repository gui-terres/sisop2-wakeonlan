#ifndef MONITORING_H
#define MONITORING_H

#include "../../stations/stations.hpp"
#include <thread>
#include <chrono>
#include <cstring>
#include <iostream>

class Monitoring {
public:
    static void requestParticipantsSleepStatus(Server &manager);
    static void waitForSleepStatusRequest(Client &client);
    static void sendWoLPacket(Server &server, string hostname);
    static void sendDowngradeToSleepyManagers(Server &manager);
};

#endif // MONITORING_H