#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include "../interface/interface.hpp"
#include "../monitoring/monitoring.hpp"
#include <thread>
#include <chrono>
#include <cstring>
#include <iostream>

class Management {
public:
    static void display(Server &server);
};

#endif // MANAGEMENT_H