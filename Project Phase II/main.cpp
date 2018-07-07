#include <iostream>
#include <thread>
#include <vector>

#include "Acceptor.h"
#include "Proposer.h"
#include "Learner.h"

using namespace std;

vector<Acceptor> acceptors;
vector<Proposer> proposers;
vector<Learner> learners;

int main(int argc, char **argv) {
    int numOfProposers = 20; // number of proposers
    int numOfAcceptors = 22; // number of acceptors
    int numOfLearners = 1; // number of learners

    unsigned int acceptorBasePort = 3000; // set the first port num of acceptors
    unsigned int proposerBasePort = 6000; // set the first port num of proposers
    unsigned int learnerBasePort = 9000; // set the first port num of learners

    vector<int> learnerPorts; // hard-code learners' port number
    for (unsigned int i = 0; i < numOfLearners; i++) learnerPorts.push_back(learnerBasePort + i);

    for (unsigned int i = 0; i < numOfAcceptors; i++) acceptors.push_back(Acceptor(i, acceptorBasePort+i, learnerPorts));
    for (unsigned int i = 0; i < numOfProposers; i++) proposers.push_back(Proposer(i, proposerBasePort+i, acceptors));
    for (unsigned int i = 0; i < numOfLearners; i++) learners.push_back((Learner(i, learnerBasePort+i, acceptors)));

    vector<thread> threads;
    int id1 = 1, id2 = 1;
    for (auto proposer : proposers) threads.push_back(thread(&Proposer::run, proposer, id1++, numOfProposers));
    for (auto learner : learners) threads.push_back(thread(&Learner::run, learner, id2++));
    for (auto acceptor : acceptors) threads.push_back(thread(&Acceptor::run, acceptor));
    for (thread& th : threads) th.join();
}
