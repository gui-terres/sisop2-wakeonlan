#ifndef MONITORING_H
#define MONITORING_H

class Monitoring {
public:
    static void requestParticipantsSleepStatus(Server &manager);
    static void waitForSleepStatusRequest(Client &client);
};

#endif // MONITORING_H