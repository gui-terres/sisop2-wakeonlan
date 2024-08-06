#include "./stations/stations.hpp"

static void Monitoring::requestParticipantsSleepStatus(Server &manager) {
    RequestData req;
    req.request = Request::SLEEP_STATUS;

    for (StationData &client : manager.getDiscoveredClients()) {
        Status status;
        if (server.requestSleepStatus(client.ipAddress, req, status) == 0) {
            client.status = status;
        }
    }
}

static void Monitoring::waitForSleepStatusRequest(Client &client) {
    client.waitForRequests();
}