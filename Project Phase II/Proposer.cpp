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

using namespace std;

Proposer::Proposer(unsigned int name, vector<Acceptor> acceptors)
        : name(name), acceptors(acceptors) {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) perror("Socket failed");
    cout << "Proposer" << name << " constructed" << endl;
}

void Proposer::propose(int val, unsigned int port) {
    default_random_engine randomEngine(val);
    uniform_int_distribution<int> uniform_dist(1, 1000);
    auto start_time = chrono::system_clock::now();
    this_thread::sleep_for(chrono::milliseconds(uniform_dist(randomEngine)));

    int majority = this->acceptors.size() / 2 + 1;
    string msg = "n: " + to_string(currPropNum) + " val: " + to_string(val);
    for (auto acceptor : this->acceptors) {
        if(uniform_dist(randomEngine) < 500) {
            memset(&servaddr, 0, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(acceptor.myport);
            servaddr.sin_addr.s_addr = INADDR_ANY;

            if (int n = sendto(sockfd, (const char *)msg, 1024,
                    0, (const struct sockaddr *) &servaddr,
                    sizeof(servaddr)) > 0) cout << "Sent to acceptor port: " << acceptor.myport << endl;

        } else cout << "Sent proposal failed" << endl;
    }
    this->currPropNum += 1;
}