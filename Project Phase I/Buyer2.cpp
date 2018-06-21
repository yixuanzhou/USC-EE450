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

#define buyer2_UDP_PORT 21876

using namespace std;

int udp_sockfd;
int tcp_sockfd;
struct sockaddr_in tcp_servaddr;
struct sockaddr_in udp_servaddr, udp_cliaddr;

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

map<string, vector<int>> parseMsg(string info) {
    cout << info << endl;
    stringstream ss(info);
    vector<string> single_info;
    map<string, vector<int>> res;
    while (ss.good()) {
        string substr;
        for (int ct = 0; ct < 5; ct++) {
            getline(ss, substr, ',');
            single_info.push_back(substr);
        }

        vector<int> val = {stoi(single_info[2]), stoi(single_info[4])};
        res[single_info[0]] = val;
        single_info.clear();
    }

    return res;
}

void tcp_buyer_client(int port) {

    tcp_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_sockfd < 0) perror("ERROR opening socket");

    bzero((char *)&tcp_servaddr, sizeof(tcp_servaddr)); // make sure the struct is empty

    tcp_servaddr.sin_family = AF_INET;
    tcp_servaddr.sin_addr.s_addr = INADDR_ANY;
    tcp_servaddr.sin_port = htons(4076);

    if (-1 == connect(tcp_sockfd, (struct sockaddr *)&tcp_servaddr, sizeof(tcp_servaddr))) perror("Connect");
}

void respond_to_agent(string seller_name) {
    char buf[1024];
    string response = "Buyer2," + seller_name;
    strcpy(buf, response.c_str());
    send(tcp_sockfd, buf, 1024, 0);
}

void udp_buyer_server(int port) {
    char ip[INET_ADDRSTRLEN];

    udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0) perror("socket creation failed");

    memset(&udp_servaddr, 0, sizeof(udp_servaddr));
    memset(&udp_cliaddr, 0, sizeof(udp_cliaddr));
    
    udp_servaddr.sin_family = AF_INET;
    udp_servaddr.sin_addr.s_addr = INADDR_ANY;
    udp_servaddr.sin_port = htons(BUYER1_UDP_PORT);

    if (bind(udp_sockfd, (const struct sockaddr *)&udp_servaddr, sizeof(udp_servaddr)) < 0) perror("bind failed");
    strcpy(ip, inet_ntoa(udp_servaddr.sin_addr));
    cout << "The <Buyer2> has UDPport " << BUYER1_UDP_PORT << " and IP address " << ip << " for Phase 1 part 2" << endl;
    
}

map<string, vector<int>> receive_from_agent() {
    char buf[1024];
    int numbytes;
    map<string, vector<int>> property_info;

    socklen_t len = sizeof(udp_cliaddr);

    if ((numbytes = recvfrom(udp_sockfd, buf, 1024, 0, (struct sockaddr *)&udp_cliaddr, &len)) > 0) {
        cout << "Received house information from <Agent1>" << endl;
    }

    property_info = parseMsg(buf);
    cout << "End of Phase I part 2 for <Buyer2>" << endl;

    close(udp_sockfd);

    return property_info;
}

 
int main() {
    udp_buyer_server(BUYER1_UDP_PORT);

    map<string, vector<int>> seller_info = receive_from_agent();
    string seller_list[] = {"sellerA", "sellerB"};

    vector<int> buyer_info = readFile("buyer/buyer2.txt");    
    int required_footage = buyer_info[1];
    int buyer_budget = buyer_info[0];

    cout << required_footage << " " << buyer_budget << endl;

    string satisfied_seller;
    int min_house_price;
    for (string seller : seller_list) {
        vector<int> inf = seller_info[seller];
        int provided_footage = inf[0];
        int house_price = inf[1];
        if (provided_footage >= required_footage and house_price <= buyer_budget) {
            if (satisfied_seller == "") { satisfied_seller = seller; min_house_price = house_price; }
            else {
                if (house_price < min_house_price) {
                    satisfied_seller = seller;
                    min_house_price = house_price;
                }
            }
        }
        cout << seller << " " << provided_footage << " " << house_price << endl;
    }

    tcp_buyer_client(4076);
    respond_to_agent(satisfied_seller);

         
    return 0;
}


