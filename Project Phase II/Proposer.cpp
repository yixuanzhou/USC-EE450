#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "Proposer.h"
#include <thread>
#include <chrono>
#include <random>
#include <sstream>

using namespace std;

Proposer::Proposer(unsigned int id, vector<Acceptor> acceptors)
        : id(id), acceptors(acceptors) {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) perror("Socket failed");
    //cout << "Proposer" << name << " constructed" << endl;
    this->majority = (unsigned int) acceptors.size() / 2 + 1;
}

Response parseResponse(string response) {
    stringstream ss(response);
    vector<string> tmp;
    while (ss.good()) {
        string str;
        getline(ss, str, ',');
        tmp.push_back(str);
    }
    Response res;
    res.type = tmp[0];
    res.result = tmp[1];
    res.promiseID = stoi(tmp[2]);
    res.acceptedID = stoi(tmp[3]);
    res.acceptedValue = stoi(tmp[4]);
    res.acceptorID = stoi(tmp[5]);
    return res;
}

string Proposer::proposal(Proposal prop) {
    string proposal;
    proposal.append(prop.type+",");
    proposal.append(to_string(prop.proposalNum)+",");
    proposal.append(to_string(prop.proposalVal)+",");
    proposal.append(to_string(prop.proposerID));
    return proposal;
}

void Proposer::run(unsigned int id) {
    bool received = false;
    this->propose(id);
    while (true) {
        do {
            received = this->receive();
        } while (!received);
        received = false;
        if (this->acceptCt >= majority) {cout << "Concensus!!!" << endl; break;}
        if (this->promiseCt >= majority) this->accept(id);
    }
}

/* Proposer wants to propose a certain value, it sends PREPARE IDp to all of Acceptors.
 * IDp must be unique, e.g. timestamp. If timeout, then resending with a higher IDp. */
void Proposer::propose(unsigned int num) {
    default_random_engine randomEngine(1000);
    uniform_int_distribution<int> uniform_dist(1, 1000);
    //auto start_time = chrono::system_clock::now();
    //this_thread::sleep_for(chrono::milliseconds(uniform_dist(randomEngine)));
    Proposal prop;
    prop.type = "prepare";
    prop.proposalNum = num;
    prop.proposerID = this->id;
    string msg = this->proposal(prop);
    for (auto acceptor : this->acceptors) {
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(acceptor.port);
        servaddr.sin_addr.s_addr = INADDR_ANY;
        sendto(sockfd, msg.c_str(), 1024, 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        cout << "Proposer" << this->id << " Sent PREPARE to Acceptor" << acceptor.id << endl;
    }
}

/* If Proposer gets a majority of PROMISE messages for a specific IDp,
 * it sends ACCEPT-REQUEST IDp and value(random) to all of Acceptors. */
void Proposer::accept(unsigned int id) {
    default_random_engine randomEngine(1000);
    uniform_int_distribution<int> uniform_dist(1, 1000);
    Proposal prop;
    prop.type = "accept";
    prop.proposalNum = this->promiseNum;
    prop.proposerID = this->id;
    for (auto acceptor : this->acceptors) {
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(acceptor.port);
        servaddr.sin_addr.s_addr = INADDR_ANY;
        /* Proposer has already got accepted value from promises */
        if (acceptor.alreadyAccepted) prop.proposalVal = acceptor.acceptedVal;
        else prop.proposalVal = uniform_dist(randomEngine); // pick random values
        string msg = this->proposal(prop);
        sendto(sockfd, msg.c_str(), 1024, 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        cout << "Proposer" << this->id << " sent ACCEPT-REQUEST to Acceptor" << acceptor.id << endl;
    }
}

bool Proposer::receive() {
    char buf[1024];
    socklen_t len = sizeof(servaddr);
    recvfrom(sockfd, (char *) buf, 1024, 0, (struct sockaddr *) &servaddr, &len);
    Response res = parseResponse(buf);
    if (res.type == "prepare" && res.result == "promise") {
        this->promiseCt++;
        this->promiseNum = res.promiseID;
    }
    if (res.type == "accept" && res.result == "chosen") this->acceptCt++;
    return true;
}