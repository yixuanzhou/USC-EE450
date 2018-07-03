#ifndef __ACCEPTOR_H__
#define __ACCEPTOR_H__
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
class Acceptor
{
public:
    Acceptor(unsigned int port);
    void accept();

private:
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
};

#endif