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
#include "Acceptor.h"
#include <vector>

using namespace std;

class Proposer
{
public:
    Proposer(unsigned int id, vector<Acceptor> acceptors);
    void propose(int val, unsigned int port);
    vector<Acceptor> acceptors;
private:
    unsigned int name;
    int sockfd;
    struct sockaddr_in servaddr;
    unsigned int currPropNum;
    unsigned int propNumInc;

};

#endif