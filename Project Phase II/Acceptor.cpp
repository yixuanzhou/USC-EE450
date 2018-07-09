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
#include "Learner.h"
#include <sstream>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>
#include <random>

using namespace std;

/* Function used to safely cout in multi-threading
 * From stackoverflow.com/questions/18277304/using-stdcout-in-multiple-threads */
class PrintThread: public ostringstream {
public:
    PrintThread() = default;
    ~PrintThread() { lock_guard<mutex> guard(_mutexPrint); cout << this->str(); }

private:
    static mutex _mutexPrint;
};

/* Acceptor constructor
 * id: Acceptor id number, e.g. Acceptor1, Acceptor2...
 * port: Acceptor UDP server port number.
 * learners: All port numbers of learner UDP clients. */
Acceptor::Acceptor(unsigned int id, unsigned int port, vector<int> learners)
        : id(id), port(port), learnerPorts(learners) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) perror("Bind failed");
}

/* Parse received request into Request struct. */
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
    req.proposerPort = stoi(tmp[4]);

    return req;
}

/* Parse Response struct into string and send to a specific client. */
bool Acceptor::respond(Response res, unsigned int port) {
    default_random_engine randomEngine(1000);
    randomEngine.seed(chrono::system_clock::now().time_since_epoch().count());
    uniform_int_distribution<int> uniform_dist(1, 1000);
    string response;
    response.append(res.type+",");
    response.append(res.result+",");
    response.append(to_string(res.promiseID)+",");
    response.append(to_string(res.acceptedID)+",");
    response.append(to_string(res.acceptedValue)+",");
    response.append(to_string(res.acceptorID));

    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(port);
    cliaddr.sin_addr.s_addr = INADDR_ANY;
    this_thread::sleep_for(chrono::milliseconds(uniform_dist(randomEngine))); // set random delay when replying to Proposer
    sendto(sockfd, response.c_str(), 1024, 0, (struct sockaddr *) &cliaddr, sizeof(cliaddr));
    return true;
}

void Acceptor::run() {
    char buf[1024];
    socklen_t len = sizeof(cliaddr);
    while (true) {
        bool received;
        bool responded;
        Request proposal;
        Response res;
        int n = -1;
        do { // PROPOSE PHASE
            n = recvfrom(sockfd, (char *) buf, 1024, 0, (struct sockaddr *) &cliaddr, &len);
            proposal = parseRequest(buf);
            if (proposal.type == "prepare")
                PrintThread{} << "Acceptor " << this->id << " received RREPARE id" << proposal.requestNum
                              << " from Proposer " << proposal.proposerID << endl;
                this_thread::sleep_for(chrono::milliseconds(1000));
            if (proposal.type == "accept")
                PrintThread{} << "Acceptor " << this->id << " received ACCEPT-REQUEST id" << proposal.requestNum
                              << ", value" << proposal.requestVal << " from Proposer " << proposal.proposerID << endl;
                this_thread::sleep_for(chrono::milliseconds(1000));
        } while (n <= 0);

        do {
            res = processProposal(proposal);
            if (res.result != "ignore") responded = this->respond(res, res.proposerPort);
            if (res.type == "prepare" && res.result == "promise")
                PrintThread{} << "Acceptor " << this->id << " has promised id" << res.promiseID << endl;
            if (res.type == "prepare" && res.result == "ignore")
                PrintThread{} << "Acceptor " << this->id << " has just ignored PROMISE" << endl;
            if (res.type == "accept" && res.result == "chosen") {
                for (unsigned int learnerPort : learnerPorts) this->respond(res, learnerPort);
                PrintThread{} << "Acceptor " << this->id << " has accepted id" << res.acceptedID << ", value" << res.acceptedValue  << endl;
            }
            if (res.type == "accept" && res.result == "ignore")
                PrintThread{} << "Acceptor " << this->id << " has just ignored ACCEPT-REQUEST" << endl;
        } while (!responded);

        //close(this->sockfd);
        //exit(0);
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
            response.proposerPort = proposal.proposerPort;
        }
        // Acceptor has not accepted any proposal yet, reply with PROMISE IDp.
        else {
            //PrintThread{} << "Acceptor" << this->id << " has promised id" << proposal.requestNum << endl;
            response.type = "prepare";
            response.result = "promise";
            response.promiseID = proposal.requestNum;
            response.acceptorID = this->id;
            response.proposerPort = proposal.proposerPort;
        }
    }
    // Acceptor ignores any request lower than IDp.
    else {
        response.type = "prepare";
        response.result = "ignore";
        response.acceptorID = this->id;
        response.proposerPort = proposal.proposerPort;
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
        response.proposerPort = proposal.proposerPort;
    }
    // ACCEPT-REQUEST ID is lower than current PROMISE ID.
    else {
        response.type = "accept";
        response.result = "ignore";
        response.acceptorID = this->id;
        response.proposerPort = proposal.proposerPort;
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