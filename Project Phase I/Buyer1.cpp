#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <arpa/inet.h>
 
#define UDP_PORT 21776

using namespace std;

string readFile(string filename) {
    string res = "";
    string buyer = filename.substr(0, 6);
    ifstream infile(filename);
    string line;
    while (getline(infile, line)) {
        stringstream iss(line);
        string s;
        while (iss.good()) {
            getline(iss, s, ':');
            res += (s+",");
        }
    }

    return buyer+","+res.substr(0, res.length()-1);
}

vector<string> parseMsg(string info) {
    stringstream ss(info);
    vector<string> res;
    while (ss.good()) {
        string substr;
        getline(ss, substr, ',');
        res.push_back(substr);
    }
    return res;
}
 
int main() {
    int sockfd;
    char buf[1024];
    struct sockaddr_in servaddr, cliaddr;
    char ip[INET_ADDRSTRLEN];
    int numbytes;

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) perror("socket creation failed");

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(UDP_PORT);

    strcpy(ip, inet_ntoa(servaddr.sin_addr));
    cout << "The <Buyer1> has UDPport " << UDP_PORT << " and IP address " << ip << " for Phase 1 part 2" << endl;

     
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 ) perror("bind failed");
     
    socklen_t len = sizeof(cliaddr);

    if ((numbytes = recvfrom(sockfd, buf, 1024, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len)) > 0) {
        cout << "Received house information from <Agent1>" << endl;
    }
    cout << "End of Phase I part 2 for <Buyer1>" << endl;
         
    return 0;
}

