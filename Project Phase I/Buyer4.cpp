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
#include <map>
#include <arpa/inet.h>

#define BUYER4_UDP_PORT 22076
#define Buyer4_TCP_PORT 4576


using namespace std;

int udp_sockfd;
int tcp_sockfd;
struct sockaddr_in tcp_servaddr, tcp_cliaddr;
struct sockaddr_in udp_servaddr, udp_cliaddr;
map <string, vector <int> > property_info;

/* Convert an integer to string
   Reused from lab2 */
string toString (int num) {
    stringstream ss;
    ss << num;
    return ss.str();
}

/* Read local txt file */
/* Reused from lab1 */
vector<int> readFile(string filename) {
    vector<int> res;
    ifstream infile;
    infile.open(filename.c_str());
    string line;
    while (getline(infile, line)) {
        stringstream iss(line);
        string s;
        int i = 0;
        while (iss.good()) {
            getline(iss, s, ':');
            if (i == 1) res.push_back(atoi(s.c_str()));
            i++;
        }
    }

    return res;
}

/* Parse message from agents */
void parseMsg(string info) {
    cout << info << endl;
    stringstream ss(info);
    vector<string> single_info;
    while (ss.good()) {
        string substr;
        for (int ct = 0; ct < 5; ct++) {
            getline(ss, substr, ',');
            single_info.push_back(substr);
        }

        vector<int> val;
        val.push_back(atoi(single_info[2].c_str()));
        val.push_back(atoi(single_info[4].c_str()));
        property_info[single_info[0]] = val;
        single_info.clear();
    }
}

void tcp_buyer_client(int port) {
    char ipaddr[INET_ADDRSTRLEN];
    int portnum;

    tcp_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_sockfd < 0) perror("ERROR opening socket");

    memset((char *)&tcp_servaddr, 0, sizeof(tcp_servaddr)); // make sure the struct is empty

    tcp_servaddr.sin_family = AF_INET;
    tcp_servaddr.sin_addr.s_addr = INADDR_ANY;
    tcp_servaddr.sin_port = htons(port);

    if (-1 == connect(tcp_sockfd, (struct sockaddr *)&tcp_servaddr, sizeof(tcp_servaddr))) perror("Connect");

    getsockname(tcp_sockfd, (struct sockaddr *) &tcp_cliaddr, (socklen_t *)sizeof(tcp_cliaddr));
    inet_ntop(AF_INET, &tcp_cliaddr.sin_addr, ipaddr, sizeof(ipaddr));
    portnum = ntohs(tcp_cliaddr.sin_port);
    printf("<Buyer4> has TCP port %d and IP address %s for Phase I part 4\n", portnum, ipaddr);
}

void respond_to_agent(string seller_name, int budget, int agent_num) {
    char buf[256];
    string response = "Buyer4,"+seller_name+","+toString(budget);
    //cout << response << endl;
    printf("<Buyer4> has sent <%s> to <Agent%d>\n", response.c_str(), agent_num);
    strcpy(buf, response.c_str());
    send(tcp_sockfd, buf, 256, 0);
}

void udp_buyer_server(int port) {
    char ip[INET_ADDRSTRLEN];
    struct hostent *he;

    udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0) perror("socket creation failed");

    memset(&udp_servaddr, 0, sizeof(udp_servaddr));
    memset(&udp_cliaddr, 0, sizeof(udp_cliaddr));
    
    udp_servaddr.sin_family = AF_INET;
    udp_servaddr.sin_addr.s_addr = INADDR_ANY;
    udp_servaddr.sin_port = htons(BUYER4_UDP_PORT);

    if (bind(udp_sockfd, (const struct sockaddr *)&udp_servaddr, sizeof(udp_servaddr)) < 0) perror("bind failed");
    if ((he = gethostbyname("nunki.usc.edu")) == NULL) perror("gethostbyname");
    cout << "The <Buyer4> has UDP port " << BUYER4_UDP_PORT << " and IP address " << inet_ntoa(*(struct in_addr*)he->h_addr) << " for Phase 1 part 2" << endl;
    
}

map <string, vector <int> > receive_from_agent(int numOfAgents) {
    char buf[1024];
    map <string, vector <int> > property_info;

    socklen_t len = sizeof(udp_cliaddr);
    for (int ct = 0; ct < numOfAgents; ct++) {
        recvfrom(udp_sockfd, buf, 1024, 0, (struct sockaddr *) &udp_cliaddr, &len);
        string agent_name = buf;
        recvfrom(udp_sockfd, buf, 1024, 0, (struct sockaddr *) &udp_cliaddr, &len);
        parseMsg(buf);
        printf("Received house information from <Agent%s>\n", agent_name.c_str());
    }
    cout << "End of Phase I part 2 for <Buyer4>" << endl;
    close(udp_sockfd);
    return property_info;
}

/* Phase I Part 4 */
void create_tcp_server() {
    char ipaddr[INET_ADDRSTRLEN];

    tcp_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_sockfd < 0) perror("ERROR opening socket");

    memset((char *) &tcp_servaddr, 0, sizeof(tcp_servaddr)); // make sure the struct is empty

    tcp_servaddr.sin_family = AF_INET;
    tcp_servaddr.sin_addr.s_addr = INADDR_ANY;
    tcp_servaddr.sin_port = htons(Buyer4_TCP_PORT);

    if (bind(tcp_sockfd, (struct sockaddr *) &tcp_servaddr, sizeof(tcp_servaddr)) < 0) perror("ERROR on binding");
    strcpy(ipaddr, inet_ntoa(tcp_servaddr.sin_addr));

    if (listen(tcp_sockfd, 5) < 0) perror("ERROR on listening");

    cout << "<Buyer4> has TCP port " << Buyer4_TCP_PORT << " and IP address " << ipaddr << " for Phase I part 4" << endl;
}

void receive_from_agent() {
    int newsockfd;
    char buffer[256];
    socklen_t clilen = sizeof(tcp_cliaddr);
    newsockfd = accept(tcp_sockfd, (struct sockaddr *) &tcp_cliaddr, &clilen);
    read(newsockfd, buffer, 255);
    printf("Will buy house from <%s>\n", buffer);
    sleep(1);
    cout << "End of Phase I part 4 for <Buyer4>" << endl;
}
 
int main() {
    udp_buyer_server(BUYER4_UDP_PORT);

    receive_from_agent(2);
    string seller_list[] = {"sellerA", "sellerB", "sellerC", "sellerD"};

    vector<int> buyer_info = readFile("buyer/buyer4.txt");    
    int required_footage = buyer_info[0];
    int buyer_budget = buyer_info[1];

    /* Iterate through all property information and compare with buyer's requirement and budget */
    string satisfied_seller = "NAK";
    int min_house_price;
    for (int i = 0; i < 4; i++) {
        string seller = seller_list[i];
        vector<int> inf = property_info[seller];
        int house_price = inf[0];
        int provided_footage = inf[1];
        if (provided_footage >= required_footage and house_price <= buyer_budget) {
            if (satisfied_seller == "NAK") { satisfied_seller = seller; min_house_price = house_price; }
            else {
                if (house_price < min_house_price) {
                    satisfied_seller = seller;
                    min_house_price = house_price;
                }
            }
        }
    }

    tcp_buyer_client(4076);
    respond_to_agent(satisfied_seller, buyer_budget, 1);
    tcp_buyer_client(4176);
    respond_to_agent(satisfied_seller, buyer_budget, 2);
    cout << "End of Phase I part 3 for <Buyer4>" << endl;
    sleep(1);

    create_tcp_server();
    receive_from_agent();
    
    return 0;
}


