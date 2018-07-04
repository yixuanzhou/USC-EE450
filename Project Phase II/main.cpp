#include <iostream>
#include <thread>
#include <random>
#include <vector>

#include "Acceptor.h"
#include "Proposer.h"

using namespace std;

vector<Acceptor> acceptors;
vector<Proposer> proposers;

int main(int argc, char **argv) {
    random_device r;
    default_random_engine e1(r());
    uniform_int_distribution<int> uniform_dist(1, 100);

    int numOfProp = 2; // number of proposers
    int numOfAcpt = 3; // number of acceptors

    for (unsigned int i = 3000; i < 3030; i+=10) acceptors.push_back(Acceptor(i, i));
    for (int i = 0; i < numOfProp; i++) proposers.push_back(Proposer(i, acceptors));

    vector<thread> proposer_threads;
    unsigned int p = 3000;
    for (auto proposer : proposers) {
        proposer_threads.push_back(thread(&Proposer::propose, proposer, 1, p));
        p += 10;
    }

    for (thread& pth : proposer_threads) pth.join();

    vector<thread> acceptor_threads;
    for (auto acceptor : acceptors) {
        acceptor_threads.push_back(thread(&Acceptor::accept, acceptor));
    }

    for (thread& ath : acceptor_threads) ath.join();
}