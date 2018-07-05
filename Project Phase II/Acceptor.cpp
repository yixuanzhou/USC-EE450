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
#include <chrono>
#include <vector>


using namespace std;

Acceptor::Acceptor(unsigned int id, unsigned int port) : id(id), port(port) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) perror("Bind failed");
    //cout << "Acceptor" << id << " created with port num: "<< port << endl;
}

Request parseRequest(string request) {
    stringstream ss(request);
    vector<string> tmp;
    while (ss.good()) {
        string str;
        getline(ss, str, ',');
        tmp.push_back(str);
    }
    Request req;
    req.type = tmp[0];
    req.requestNum = stoi(tmp[1]);
    req.requestVal = stoi(tmp[2]);
    req.proposerID = stoi(tmp[3]);

    return req;
}

string Acceptor::respond(Response res) {
    string response;
    response.append(res.type+",");
    response.append(res.result+",");
    response.append(to_string(res.promiseID)+",");
    response.append(to_string(res.acceptedID)+",");
    response.append(to_string(res.acceptedValue)+",");
    response.append(to_string(res.acceptorID));
    return response;
}

void Acceptor::run() {
    char buf[1024];
    socklen_t len = sizeof(cliaddr);
    while (true) {
        recvfrom(sockfd, (char *) buf, 1024, 0, (struct sockaddr *) &cliaddr, &len);
        Request proposal = parseRequest(buf);
        cout << "Acceptor" << this->id <<" received proposal: " << proposal.requestNum << endl;
        Response res = processProposal(proposal);
        string response = respond(res);
        sendto(sockfd, response.c_str(), 1024, 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
        recvfrom(sockfd, (char *) buf, 1024, 0, (struct sockaddr *) &cliaddr, &len);
    }
}

/* Acceptor receives a PREPARE message for IDp:
 * if it has PROMISED to ignore request with this IDp, then ignore,
 * otherwise, it PROMISE to ignore any request lower than IDp. */
Response Acceptor::prepare(Request proposal) {
    Response response;

    if (this->minNumToAccept < proposal.requestNum) {
        this->minNumToAccept = proposal.requestNum;
        // Acceptor has already accepted IDa, then reply with PROMISE IDp, accepted IDa, val.
        if (this->alreadyAccepted) {
            response.type = "prepare";
            response.result = "promise";
            response.promiseID = proposal.requestNum;
            response.acceptedID = this->acceptedNum;
            response.acceptedValue = this->acceptedVal;
            response.acceptorID = this->id;
        }
        // Acceptor has not accepted any proposal yet, reply with PROMISE IDp.
        else {
            response.type = "prepare";
            response.result = "promise";
            response.promiseID = proposal.requestNum;
            response.acceptorID = this->id;
        }
    }
    // Acceptor ignores any request lower than IDp
    else {
        response.type = "prepare";
        response.result = "reject";
        response.acceptorID = this->id;
    }

    return response;
}

/* Acceptor receives an ACCEPT-REQUEST message for IDp, value:
 * if it has PROMISED to ignore request with this IDp, then ignore,
 * otherwise, reply with ACCEPT IDp, value. Also send it to learners. */
Response Acceptor::accept(Request proposal) {
    Response response;

    if (this->minNumToAccept == proposal.requestNum) {
        this->alreadyAccepted = true;
        this-> acceptedNum = proposal.requestNum;
        this-> acceptedVal = proposal.requestVal;
        response.type = "accept";
        response.result = "chosen";
        response.acceptedID = proposal.requestNum;
        response.acceptedValue = proposal.requestVal;
        response.acceptorID = this->id;
    } else {
        response.type = "accept";
        response.result = "reject";
        response.acceptorID = this->id;
    }

    return response;
}

Response Acceptor::processProposal(Request proposal) {
    Response response;
    if (proposal.type == "prepare") response = this->prepare(proposal);
    else if (proposal.type == "accept") response = this->accept(proposal);
    else cout << "Invalid proposal" << endl;
    return response;
}