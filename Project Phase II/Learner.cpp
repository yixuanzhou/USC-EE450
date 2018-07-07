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
#include <sstream>
#include <mutex>
#include "Learner.h"
#include "Acceptor.h"

using namespace std;

class PrintThread: public ostringstream {
public:
    PrintThread() = default;
    ~PrintThread() { lock_guard<mutex> guard(_mutexPrint); cout << this->str(); }

private:
    static mutex _mutexPrint;
};

Learner::Learner(unsigned int id, unsigned int port, vector<Acceptor> acceptors)
                 : id(id), port(port), acceptors(acceptors) {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) perror("Socket failed");
    memset(&cliaddr, 0, sizeof(cliaddr));
    cliaddr.sin_family = AF_INET;
    cliaddr.sin_port = htons(port);
    cliaddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (const struct sockaddr *)&cliaddr, sizeof(cliaddr)) < 0) perror("Bind failed");
}

Response Learner::parseResponse(string response) {
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

void Learner::run(unsigned int id) {
    bool reachConsensus = false;
    do {
        this->receive();
        if (this->acceptCt >= majority) {cout << "Learner reaches CONSENSUS!!!" << endl; reachConsensus = true;}
    } while (!reachConsensus);

    close(this->sockfd);
    exit(0);
}

void Learner::receive() {
    char buf[1024];
    socklen_t len = sizeof(servaddr);
    recvfrom(sockfd, (char *) buf, 1024, 0, (struct sockaddr *) &servaddr, &len);

    Response res = parseResponse(buf);
    if (res.type == "accept" && res.result == "chosen") {
        this->acceptCt++;
        this->acceptedVal = res.acceptedValue;
        PrintThread{} << "Learner " << this->id << " received ACCEPT id" << res.acceptedID << " from Acceptor " << res.acceptorID << endl;
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}
