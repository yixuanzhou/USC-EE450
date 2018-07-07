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

struct Proposal
{
    string type;
    unsigned int proposalNum = 0;
    int proposalVal = -1;
    unsigned int proposerID;
};

class Proposer
{
public:
    unsigned int id;
    unsigned int port;
    Proposer(unsigned int id, unsigned int port, vector<Acceptor> acceptors);
    void run(unsigned int id, unsigned int numOfProposer);
    vector<Acceptor> acceptors;

private:
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    unsigned int promiseNum;
    void propose(unsigned int id);
    void accept(unsigned int id);
    bool receive();
    string proposal(Proposal prop);
    unsigned int acceptCt = 0;
    unsigned int promiseCt = 0;
    int acceptedVal;
    unsigned int majority;
};
#endif