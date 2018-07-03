#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Proposer.h"

using namespace std;

Proposer::Proposer() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    cout << "Proposer constructed" << endl;
}

void Proposer::propose(unsigned int port) {

    char *hello = "Hello from client";
    for (int i = port; i < port + 3 * 10; i+=10) {
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(i);
        servaddr.sin_addr.s_addr = INADDR_ANY;

        if (int n = sendto(sockfd, (const char *)hello, 1024,
                0, (const struct sockaddr *) &servaddr,
                sizeof(servaddr)) > 0) cout << "Sent to acceptor port: " << i << endl;

    }
}