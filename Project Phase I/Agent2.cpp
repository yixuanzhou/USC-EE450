#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>

#define Agent2_TCP_PORT 4176

using namespace std;

int udp_sockfd;
int tcp_sockfd;
struct sockaddr_in tcp_servaddr, tcp_cliaddr;
struct sockaddr_in udp_servaddr;

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

string parse_seller_info(string filename) {
    string res = "";
    ifstream infile(filename);
    string line;
    while (getline(infile, line)) {
        stringstream iss(line);
        string s;
        while (iss.good()) {
            getline(iss, s, ',');
            res += (s+",");
        }
    }
    return res.substr(0, res.length()-1);
}

void clear_seller_info(string filename) {
    ofstream ofs;
    ofs.open(filename, ofstream::out | ofstream::trunc);
    ofs.close();
}

string store_seller_info(string seller_info) {
    string seller_name = seller_info.substr(0,7);
    ofstream res;
    res.open("Agent2.txt", ios::out | ios::app);
    res << seller_info;
    res << endl;
    res.close();

    return seller_name;
}

void store_buyer_info(string buyer_info) {
    ofstream res;
    res.open("Agent2.txt", ios::out | ios::app);
    res << buyer_info;
    res << endl;
    res.close();
}

vector<vector<string>> parse_buyer_info(string filename) {
    vector<vector<string>> res;
    ifstream infile(filename);
    vector<string> info;
    string line;
    while (getline(infile, line)) {
        stringstream iss(line);
        string s;
        while (iss.good()) {
            getline(iss, s, ',');
            info.push_back(s);
        }
        res.push_back(info);
        info.clear();
    }
    return res;
}

/* Phase I part 1 & 3*/
void tcp_server(int port, int partNum) {
    char ipaddr[INET_ADDRSTRLEN];

    tcp_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_sockfd < 0) perror("ERROR opening socket");

    memset((char *) &tcp_servaddr, 0, sizeof(tcp_servaddr)); // make sure the struct is empty

    tcp_servaddr.sin_family = AF_INET;
    tcp_servaddr.sin_addr.s_addr = INADDR_ANY;
    tcp_servaddr.sin_port = htons(Agent2_TCP_PORT);

    if (bind(tcp_sockfd, (struct sockaddr *) &tcp_servaddr, sizeof(tcp_servaddr)) < 0) perror("ERROR on binding");
    strcpy(ipaddr, inet_ntoa(tcp_servaddr.sin_addr));
    printf("<Agent2> has TCP port %d and IP address %s for Phase I part %d\n", Agent2_TCP_PORT, ipaddr, partNum);
    if (listen(tcp_sockfd, 5) < 0) perror("ERROR on listening");
}

/* Phase I part 1 */
void receive_from_sellers(int numOfClients) {
    int newsockfd, pid;
    char buffer[256];
    int numbytes;

    socklen_t clilen = sizeof(tcp_cliaddr);

    for(int ct = 0; ct < numOfClients; ct++) {
        newsockfd = accept(tcp_sockfd, (struct sockaddr *) &tcp_cliaddr, &clilen);
        if (newsockfd < 0) perror("ERROR on accept");

        pid = fork();
        if (pid < 0) perror("ERROR on fork");
        if (pid == 0) {
            close(tcp_sockfd);
            numbytes = read(newsockfd, buffer, 255);
            if (numbytes < 0) perror("ERROR reading from socket");
            string seller_name = store_seller_info(string(buffer));
            printf("Received house information from <%s>\n", seller_name.c_str());
            exit(0);
        } else close(newsockfd);
    }
    sleep(1);
    cout << "End of Phase I part 1 for <Agent2>" << endl;
}

/* Phase I part 2 */
void create_udp_socket() {
    udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd == -1) perror("socket");
    //printf("<Agent1> has UDP port %d and IP address %s for Phase I part 2\n", Agent1_TCP_PORT, ipaddr);
}

/* Phase I part 2 */
void send_to_buyer(int port, int buyer_name) {
    char buf[1024];

    memset(&udp_servaddr, 0, sizeof(udp_servaddr));

    udp_servaddr.sin_family = AF_INET;
    udp_servaddr.sin_port = htons(port);
    udp_servaddr.sin_addr.s_addr = INADDR_ANY;
    socklen_t addrlen = sizeof(udp_servaddr);
    sendto(udp_sockfd, "1", 1024, 0, (const struct sockaddr *) &udp_servaddr, sizeof(udp_servaddr));
    string msg = parse_seller_info("Agent2.txt");
    strcpy(buf, msg.c_str());
    sendto(udp_sockfd, buf, 1024, 0, (const struct sockaddr *) &udp_servaddr, sizeof(udp_servaddr));
    printf("<Agent2> has sent %s to <Buyer%d>\n", buf, buyer_name);
}

/* Phase I part 3 */
void receive_from_buyers(int numOfBuyers) {
    int newsockfd, pid;
    char buffer[256];
    int numbytes;
    bzero(buffer, 256);
    socklen_t clilen = sizeof(tcp_cliaddr);

    for(int ct = 0; ct < numOfBuyers; ct++) {
        newsockfd = accept(tcp_sockfd, (struct sockaddr *) &tcp_cliaddr, &clilen);
        if (newsockfd < 0) perror("ERROR on accept");

        pid = fork();
        if (pid < 0) perror("ERROR on fork");
        if (pid == 0) {
            close(tcp_sockfd);
            numbytes = read(newsockfd, buffer, 255);
            if (numbytes < 0) perror("ERROR reading from socket");
            store_buyer_info(string(buffer));
            string response = string(buffer);
            vector<string> msg = parseMsg(response);
            printf("<Agent2> receives the offer from <%s>\n", msg[0].c_str());
            exit(0);
        } else close(newsockfd);
    }
    sleep(1);
    cout << "End of Phase I part 3 for <Agent2>" << endl;
}

/* Phase I part 4 */
vector<vector<string>> calculate_bid() {
    vector<vector<string>> bids = parse_buyer_info("Agent2.txt");
    vector<string> res = {"NAK", "NAK"};
    vector<string> interested_buyers;
    int bidC = 0;
    int bidD = 0;
    for (int i = 0; i < 5; i++) {
        vector<string> bid = bids[i];
        if (bid[1] == "sellerC") {
            interested_buyers.push_back(bid[0]);
            int curr_bid = stoi(bid[2]);
            if (curr_bid > bidC) {
                res[0] = bid[0];
                bidC = curr_bid;
            }
        }
        if (bid[1] == "sellerD") {
            int curr_bid = stoi(bid[2]);
            interested_buyers.push_back(bid[0]);
            if (curr_bid > bidD) {
                res[1] = bid[0];
                bidD = curr_bid;
            }
        }
    }

    vector<vector<string>> inf;
    inf.push_back(res);
    inf.push_back(interested_buyers);

    return inf;
}

/* Phase I part 4 */
void send_final_result(int port, string name, string result) {
    char buf[1024];
    int numbyte;
    char ipaddr[INET_ADDRSTRLEN];

    int tcp_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_sockfd == -1) perror("Socket");
    bzero((void *) &tcp_servaddr, sizeof(tcp_servaddr));
    tcp_servaddr.sin_family = AF_INET;
    tcp_servaddr.sin_port = htons(port);
    tcp_servaddr.sin_addr.s_addr = INADDR_ANY;
    strcpy(ipaddr, inet_ntoa(tcp_servaddr.sin_addr));
    //printf("<SellerA> has TCP port %d and IP address %s for Phase I part 1\n", Agent1_TCP_PORT, ipaddr);

    if (-1 == connect(tcp_sockfd, (struct sockaddr *)&tcp_servaddr, sizeof(tcp_servaddr))) perror("Connect");
    strcpy(buf, result.c_str());
    send(tcp_sockfd, buf, 1024, 0);
    printf("<Agent2> has send the result to <%s>\n", name.c_str());
    sleep(1);
}

int main(int argc, char *argv[]) {

    tcp_server(Agent2_TCP_PORT, 1);
    receive_from_sellers(2);
    create_udp_socket();
    send_to_buyer(21776, 1);
    send_to_buyer(21876, 2);
    send_to_buyer(21976, 3);
    send_to_buyer(22076, 4);
    send_to_buyer(22176, 5);
    clear_seller_info("Agent2.txt");
    cout << "End of Phase I part 2 for <Agent2>" << endl;
    close(udp_sockfd);
    //tcp_server(Agent1_TCP_PORT, 3);
    //vector<pair<string, string>> res;
    receive_from_buyers(5);
    vector<vector<string>> inf = calculate_bid();
    vector<string> to_sellers = inf[0];
    vector<string> to_buyers = inf[1];
    clear_seller_info("Agent2.txt");

    map<string, string> lmao;
    send_final_result(3876, "sellerC", to_sellers[0]);
    lmao[to_sellers[0]] = "sellerC";
    send_final_result(3976, "sellerD", to_sellers[1]);
    lmao[to_sellers[1]] = "sellerD";
    map<string, int> all_buyers_port_num;
    all_buyers_port_num["Buyer1"]  = 4276;
    all_buyers_port_num["Buyer2"]  = 4376;
    all_buyers_port_num["Buyer3"]  = 4476;
    all_buyers_port_num["Buyer4"]  = 4576;
    all_buyers_port_num["Buyer5"]  = 4676;

    for (string msg : to_buyers) {
        int buyer_port_num = all_buyers_port_num[msg];
        if (msg != to_sellers[0] and msg != to_sellers[1]) send_final_result(buyer_port_num, msg, "NAK");
        else send_final_result(buyer_port_num, msg, lmao[msg]);
    }

    return 0;
}
