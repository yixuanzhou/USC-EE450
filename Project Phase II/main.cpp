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

    for (unsigned int i = 0; i < numOfAcpt; i++) acceptors.push_back(Acceptor(i, 3000+i));
    for (int i = 0; i < numOfProp; i++) proposers.push_back(Proposer(i, acceptors));

    vector<thread> threads;
    unsigned int id = 1;
    for (auto proposer : proposers) threads.push_back(thread(&Proposer::run, proposer, id++));

    //for (thread& pth : proposer_threads) pth.join();

    //vector<thread> acceptor_threads;
    for (auto acceptor : acceptors) threads.push_back(thread(&Acceptor::run, acceptor));


    for (thread& th : threads) th.join();
}
