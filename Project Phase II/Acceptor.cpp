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
#include <sstream>

using namespace std;

Acceptor::Acceptor(unsigned int id, unsigned int port) : id(id), myport(port) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) perror("Bind failed");
    cout << "Acceptor" << id << " created with port num: "<< port << endl;
}

vector<int> Acceptor::parseMsg(string msg) {
    stringstream ss(msg);
    vector<int> res;
    while (ss.good()) {
        string str;
        getline(ss, str, ',');
        res.push_back(stoi(str));
    }
    return res;
}

void Acceptor::prepare() {
    char buf[128];
    socklen_t len = sizeof(cliaddr);
    recvfrom(sockfd, (char *)buf, 128, 0, (struct sockaddr *) &cliaddr, &len);
    vector<int> msg = this->parseMsg(buf);
    unsigned int currProposalNum = (unsigned int) msg[0];
    int currProposalVal = msg[1];
    AcceptorResponse res;

    // if acceptor never receives any proposal, then
    if (this->state.lastProposalNum == 0 && this->state.minNumToAccept == 0) {
        this->state.lastProposalNum = currProposalNum;
        this->state.lastProposalVal = currProposalVal;
        this->state.minNumToAccept = currProposalNum;
        res.prepareAck = true;
        res.accepted = true;
        res.prevValue = currProposalVal;
        res.prevProposalNum = currProposalNum;
    } else {
        // if acceptor received proposal that is greater than the promised proposal
        if (this->state.minNumToAccept < currProposalNum) {
            this->state.minNumToAccept = currProposalNum;

        } else { // if acceptor received proposal that is equal to the promised proposal
            if (this->state.minNumToAccept == currProposalNum) {
                this->state.lastProposalNum = currProposalNum;
                this->state.lastProposalVal = currProposalVal;
            } else { // if acceptor received proposal that is less than the promised proposal

            }

        }
    }



    if (proposalNum > minNumToAccept) {
        minNumToAccept = proposalNum;
        prevValue = value;
    } else prepareAck = false;
}

void Acceptor::accept() {


}