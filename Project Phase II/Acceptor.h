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

struct Response
{
    string type;
    string result;
    unsigned int promiseID = 0;
    unsigned int acceptedID = 0;
    int acceptedValue = -1;
    unsigned int acceptorID;
    unsigned int proposerPort;
};

struct Request
{
    string type;
    unsigned int requestNum = 0;
    int requestVal = -1;
    unsigned int proposerID;
    unsigned int proposerPort;
};

class Acceptor
{
public:
    unsigned int id;
    unsigned int port;
    bool alreadyAccepted = false;
    int acceptedVal;
    Acceptor(unsigned int id, unsigned int port, vector<int> learnerPorts);
    void run();
    Response prepare(Request proposal);
    Response accept(Request proposal);

private:
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    bool respond(Response res, unsigned int port);
    Response processProposal(Request proposal);
    unsigned int acceptedNum;
    unsigned int minNumToAccept = 0;
    vector<int> learnerPorts;
};
#endif