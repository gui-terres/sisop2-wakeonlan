#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "./discovery.h"

#define PORT 4000
#define BUFFER_SIZE 256

using namespace std;

namespace Client {
    int getClientHostname(char *buffer, size_t bufferSize, string &hostname) {
        memset(buffer, 0, sizeof(buffer));

        if ((gethostname(buffer, bufferSize)) == -1) {
            cout << "ERROR on getting the hostname." << endl;
            return -1;
        }

        hostname.assign(buffer);
        memset(buffer, 0, sizeof(buffer));

        return 0;
    }

    int socket() {}
}