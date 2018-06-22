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
#include <regex>
#include <arpa/inet.h>

#define BUYER5_UDP_PORT 22176
#define Buyer5_TCP_PORT 4676


using namespace std;

int udp_sockfd;
int tcp_sockfd;
struct sockaddr_in tcp_servaddr, tcp_cliaddr;
struct sockaddr_in udp_servaddr, udp_cliaddr;
map<string, vector<int>> property_info;

vector<int> readFile(string filename) {
    vector<int> res;
    ifstream infile(filename);
    string line;
    while (getline(infile, line)) {
        stringstream iss(line);
        string s;
        while (iss.good()) {
            getline(iss, s, ':');
            if (regex_match(s, regex("[0-9]+"))) res.push_back(stoi(s));
        }
    }

    return res;
}

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

        vector<int> val = {stoi(single_info[2]), stoi(single_info[4])};
        property_info[single_info[0]] = val;
        single_info.clear();
    }
}

void tcp_buyer_client(int port) {

    tcp_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_sockfd < 0) perror("ERROR opening socket");

    bzero((char *)&tcp_servaddr, sizeof(tcp_servaddr)); // make sure the struct is empty

    tcp_servaddr.sin_family = AF_INET;
    tcp_servaddr.sin_addr.s_addr = INADDR_ANY;
    tcp_servaddr.sin_port = htons(port);

    if (-1 == connect(tcp_sockfd, (struct sockaddr *)&tcp_servaddr, sizeof(tcp_servaddr))) perror("Connect");
}

void respond_to_agent(string seller_name, int budget) {
    char buf[256];
    string response = "Buyer5,"+seller_name+","+to_string(budget);
    cout << response << endl;
    strcpy(buf, response.c_str());
    send(tcp_sockfd, buf, 256, 0);
}

void udp_buyer_server(int port) {
    char ip[INET_ADDRSTRLEN];

    udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0) perror("socket creation failed");

    memset(&udp_servaddr, 0, sizeof(udp_servaddr));
    memset(&udp_cliaddr, 0, sizeof(udp_cliaddr));
    
    udp_servaddr.sin_family = AF_INET;
    udp_servaddr.sin_addr.s_addr = INADDR_ANY;
    udp_servaddr.sin_port = htons(BUYER5_UDP_PORT);

    if (bind(udp_sockfd, (const struct sockaddr *)&udp_servaddr, sizeof(udp_servaddr)) < 0) perror("bind failed");
    strcpy(ip, inet_ntoa(udp_servaddr.sin_addr));
    cout << "The <Buyer5> has UDP port " << BUYER5_UDP_PORT << " and IP address " << ip << " for Phase 1 part 2" << endl;
    
}

map<string, vector<int>> receive_from_agent(int numOfAgents) {
    char buf[1024];
    map<string, vector<int>> property_info;

    socklen_t len = sizeof(udp_cliaddr);
    for (int ct = 0; ct < numOfAgents; ct++) {
        recvfrom(udp_sockfd, buf, 1024, 0, (struct sockaddr *) &udp_cliaddr, &len);
        string agent_name = buf;
        recvfrom(udp_sockfd, buf, 1024, 0, (struct sockaddr *) &udp_cliaddr, &len);
        parseMsg(buf);
        printf("Received house information from <Agent%s>\n", agent_name.c_str());
    }
    cout << "End of Phase I part 2 for <Buyer5>" << endl;
    close(udp_sockfd);
    return property_info;
}

/* Phase I Part 4 */
void create_tcp_server() {
    char ipaddr[INET_ADDRSTRLEN];

    tcp_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_sockfd < 0) perror("ERROR opening socket");

    bzero((char *) &tcp_servaddr, sizeof(tcp_servaddr)); // make sure the struct is empty

    tcp_servaddr.sin_family = AF_INET;
    tcp_servaddr.sin_addr.s_addr = INADDR_ANY;
    tcp_servaddr.sin_port = htons(Buyer5_TCP_PORT);

    if (bind(tcp_sockfd, (struct sockaddr *) &tcp_servaddr, sizeof(tcp_servaddr)) < 0) perror("ERROR on binding");
    strcpy(ipaddr, inet_ntoa(tcp_servaddr.sin_addr));

    if (listen(tcp_sockfd, 5) < 0) perror("ERROR on listening");

    cout << "<Buyer5> has TCP port " << Buyer5_TCP_PORT << " and IP address " << ipaddr << " for Phase I part 4" << endl;
}

void receive_from_agent() {
    int newsockfd;
    char buffer[256];
    socklen_t clilen = sizeof(tcp_cliaddr);
    newsockfd = accept(tcp_sockfd, (struct sockaddr *) &tcp_cliaddr, &clilen);
    read(newsockfd, buffer, 255);
    cout << buffer << endl;
    sleep(1);
    cout << "End of Phase I part 4 for <Buyer5>" << endl;
}

 
int main() {
    udp_buyer_server(BUYER5_UDP_PORT);

    receive_from_agent(2);
    string seller_list[] = {"sellerA", "sellerB", "sellerC", "sellerD"};

    vector<int> buyer_info = readFile("buyer/buyer5.txt");    
    int required_footage = buyer_info[0];
    int buyer_budget = buyer_info[1];

    //cout << required_footage << " " << buyer_budget << endl;

    string satisfied_seller = "NAK";
    int min_house_price;
    for (string seller : seller_list) {
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
        //cout << seller << " " << provided_footage << " " << house_price << endl;
    }

    tcp_buyer_client(4076);
    respond_to_agent(satisfied_seller, buyer_budget);
    tcp_buyer_client(4176);
    respond_to_agent(satisfied_seller, buyer_budget);

    create_tcp_server();
    receive_from_agent();
    return 0;
}


