#ifndef PAXOS_LEARNER_H
#define PAXOS_LEARNER_H

#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>
#include "Acceptor.h"

using namespace std;

class Learner
{
public:
    unsigned int id;
    unsigned int port;
    Learner(unsigned int id, unsigned int port, vector<Acceptor> acceptors);
    void run(unsigned int id);
    vector<Acceptor> acceptors;

private:
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    int acceptedVal;
    unsigned int majority;
    unsigned int acceptCt = 0;
    void receive();
    Response parseResponse(string Response);
};

#endif //PAXOS_LEARNER_H
