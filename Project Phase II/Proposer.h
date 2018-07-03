#ifndef __PROPOSER_H__
#define __PROPOSER_H__
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

class Proposer
{
public:
    Proposer();
    void propose(unsigned int port);
private:
    int sockfd;
    struct sockaddr_in servaddr;
};

#endif