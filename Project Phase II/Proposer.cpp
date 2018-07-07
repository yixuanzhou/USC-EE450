#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <thread>
#include <chrono>
#include <random>
#include <sstream>
#include <mutex>

#include "Proposer.h"

using namespace std;

class PrintThread: public ostringstream {
public:
    PrintThread() = default;
    ~PrintThread() { lock_guard<mutex> guard(_mutexPrint); cout << this->str(); }

private:
    static mutex _mutexPrint;
};

mutex PrintThread::_mutexPrint{};

/* Proposer constructor */
Proposer::Proposer(unsigned int id, unsigned int port, vector<Acceptor> acceptors)
                   : id(id), port(port), acceptors(acceptors) {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) perror("Socket failed");
    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(port);
    cliaddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (const struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0) perror("Bind failed");
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
    proposal.append(to_string(prop.proposerID)+",");
    proposal.append(to_string(this->port));
    return proposal;
}

void Proposer::run(unsigned int id, unsigned int numOfProposer) {
    bool endPropose = false;
    bool reachConsensus = false;

    while (true) {
        do { // PROPOSE PHASE
            this->propose(id+=numOfProposer); // Proposer sends proposal to Acceptors
            this_thread::sleep_for(chrono::milliseconds(1000));
            //id += numOfProposer;
            while (true) {
                if (!this->receive()) break;
                if (this->promiseCt >= majority) { endPropose = true; break; }
            }
        } while (!endPropose);

        do { // ACCEPT-REQUEST PHASE
            this->accept(id);
            this_thread::sleep_for(chrono::milliseconds(1000));
            while (true) {
                if (!this->receive()) break;
                if (this->acceptCt >= majority) {cout << "Reach CONSENSUS with value " << acceptedVal << endl; reachConsensus = true;}
            }
        } while (!reachConsensus);

        close(this->sockfd);
        exit(0);

        /* If a majority of Acceptors accept a request with ID and value,
         * consensus has been reached and is that value. */
    }
}

/* Proposer wants to propose a certain value, it sends PREPARE IDp to all of Acceptors.
 * IDp must be unique, e.g. timestamp. If timeout, then resending with a higher IDp. */
void Proposer::propose(unsigned int num) {
    default_random_engine randomEngine(1000);
    uniform_int_distribution<int> uniform_dist(1, 1000);
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
        PrintThread{} << "Proposer " << this->id << " sent PREPARE id" << num << " to Acceptor " << acceptor.id << endl;
    }
}

/* If Proposer gets a majority of PROMISE messages for a specific IDp,
 * it sends ACCEPT-REQUEST IDp and value(random) to all of Acceptors. */
void Proposer::accept(unsigned int id) {
    default_random_engine randomEngine(1000);
    randomEngine.seed(chrono::system_clock::now().time_since_epoch().count());
    uniform_int_distribution<int> uniform_dist(1, 1000);
    int randomVal = uniform_dist(randomEngine);
    Proposal prop;
    prop.type = "accept";
    prop.proposalNum = this->promiseNum;
    prop.proposerID = this->id;
    for (auto acceptor : this->acceptors) {
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(acceptor.port);
        servaddr.sin_addr.s_addr = INADDR_ANY;
        // Proposer has already got accepted value from promises.
        if (acceptor.alreadyAccepted) {cout << "caoniam" << endl; prop.proposalVal = acceptor.acceptedVal;}
        else prop.proposalVal = randomVal; // else it picks a random value
        string msg = this->proposal(prop);
        sendto(sockfd, msg.c_str(), 1024, 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        PrintThread{} << "Proposer " << this->id << " sent ACCEPT-REQUEST id" << prop.proposalNum
                      << ", value" << prop.proposalVal <<" to Acceptor " << acceptor.id << endl;
    }
}

bool Proposer::receive() {
    char buf[1024];
    socklen_t len = sizeof(servaddr);
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    if (recvfrom(sockfd, (char *) buf, 1024, 0, (struct sockaddr *) &servaddr, &len) < 0) {cout<< "TO" << endl; return false;}

    Response res = parseResponse(buf);
    if (res.type == "prepare" && res.result == "promise") {
        this->promiseCt++;
        this->promiseNum = res.promiseID;
        // Proposer knows Acceptor has already accepted a value, mark this acceptor as "accepted"
        if (res.acceptedValue > 0) {
            this->acceptors[res.acceptorID].alreadyAccepted = true;
            this->acceptors[res.acceptorID].acceptedVal = res.acceptedValue;
        }
        PrintThread{} << "Proposer " << this->id << " received PROMISE id" << res.promiseID << " from Acceptor " << res.acceptorID << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
    if (res.type == "accept" && res.result == "chosen") {
        this->acceptCt++;
        this->acceptedVal = res.acceptedValue;
        this->acceptors[res.acceptorID].alreadyAccepted = true;
        PrintThread{} << "Proposer " << this->id << " received ACCEPT id" << res.acceptedID << " from Acceptor " << res.acceptorID << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
    return true;
}