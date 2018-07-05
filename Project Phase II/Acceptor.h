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


using namespace std;

struct Response
{
    string type;
    string result;
    unsigned int promiseID = 0;
    unsigned int acceptedID = 0;
    int acceptedValue = -1;
    unsigned int acceptorID;
};

struct Request
{
    string type;
    unsigned int requestNum = 0;
    int requestVal = -1;
    unsigned int proposerID;
};

class Acceptor
{
public:
    unsigned int id;
    unsigned int port;
    bool alreadyAccepted;
    int acceptedVal;
    Acceptor(unsigned int id, unsigned int port);
    void run();
    Response prepare(Request proposal);
    Response accept(Request proposal);

private:

    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    string respond(Response res);
    Response processProposal(Request proposal);
    unsigned int acceptedNum;
    unsigned int minNumToAccept = 0;
};
#endif