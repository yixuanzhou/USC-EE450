#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Acceptor.h"

using namespace std;

Acceptor::Acceptor(unsigned int port) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) perror("Bind failed");
    cout << "Acceptor created with port num: "<< port << endl;
}

void Acceptor::accept() {
    char buffer[1024];
    cout << "caonima" << endl;
    socklen_t len = sizeof(cliaddr);
    if (int n = recvfrom(sockfd, (char *)buffer, 1024,
                 0, (struct sockaddr *) &cliaddr,
                 &len)> 0) cout << "dd" << endl;
}