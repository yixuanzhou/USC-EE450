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
#include <vector>

using namespace std;

struct AcceptorResponse
{
    bool prepareAck;
    bool accepted;
    int prevValue;
    unsigned int prevProposalNum;
};

struct AcceptorState
{
    unsigned int lastProposalNum;
    int lastProposalVal;
    unsigned int minNumToAccept;
};

class Acceptor
{
public:
    unsigned int myport;
    Acceptor(unsigned int id, unsigned int port);
    void prepare();
    void accept();

private:
    unsigned int id;
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    vector<int> parseMsg(string msg);
    AcceptorState state;
};

#endif