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
struct sockaddr_in udp_servaddr, udp_cliaddr;

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

/* Parse information from sellers */
string parse_seller_info(string filename) {
    string res = "";
    ifstream infile;
    infile.open(filename.c_str());
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

/* Empty the temporary local txt file */ 
void clear_seller_info(string filename) {
    ofstream ofs;
    ofs.open(filename.c_str(), ofstream::out | ofstream::trunc);
    ofs.close();
}

/* Store seller information in a local temporary txt file */
string store_seller_info(string seller_info) {
    string seller_name = seller_info.substr(0,7);
    ofstream res;
    res.open("Agent2.txt", ios::out | ios::app);
    res << seller_info;
    res << endl;
    res.close();

    return seller_name;
}

/* Store buyer information in a local temporary txt file */
void store_buyer_info(string buyer_info) {
    ofstream res;
    res.open("Agent2.txt", ios::out | ios::app);
    res << buyer_info;
    res << endl;
    res.close();
}

/* Parse information from buyers */
vector <vector <string> > parse_buyer_info(string filename) {
    vector< vector <string> > res;
    ifstream infile;
    infile.open(filename.c_str());
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
void tcp_server(int port, int part) {
    struct hostent *he;
    char ipaddr[INET_ADDRSTRLEN];

    tcp_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_sockfd < 0) perror("ERROR opening socket");

    memset((char *) &tcp_servaddr, 0, sizeof(tcp_servaddr)); // make sure the struct is empty

    tcp_servaddr.sin_family = AF_INET;
    tcp_servaddr.sin_addr.s_addr = INADDR_ANY;
    tcp_servaddr.sin_port = htons(Agent2_TCP_PORT);

    if ((he = gethostbyname("nunki.usc.edu")) == NULL) perror("gethostbyname");

    if (bind(tcp_sockfd, (struct sockaddr *) &tcp_servaddr, sizeof(tcp_servaddr)) < 0) perror("ERROR on binding");
    printf("<Agent2> has TCP port %d and IP address %s for Phase I part %d\n", Agent2_TCP_PORT, inet_ntoa(*(struct in_addr*)he->h_addr), part);
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
    char ipaddr[INET_ADDRSTRLEN];
    memset(&udp_servaddr, 0, sizeof(udp_servaddr));

    struct hostent *he;
    if ((he = gethostbyname("localhost")) == NULL) perror("gethostbyname");
    struct in_addr **addr_list = (struct in_addr **)he->h_addr_list;

    udp_servaddr.sin_family = AF_INET;
    udp_servaddr.sin_port = htons(port);
    udp_servaddr.sin_addr.s_addr = INADDR_ANY;

    socklen_t addrlen = sizeof(udp_servaddr);
    sendto(udp_sockfd, "2", 1024, 0, (const struct sockaddr *) &udp_servaddr, sizeof(udp_servaddr));
    memset(&udp_cliaddr, 0, sizeof(udp_cliaddr));
    getsockname(udp_sockfd, (struct sockaddr *) &udp_cliaddr, (socklen_t *)sizeof(udp_cliaddr));
    //printf("<Agent2> has UDP port %d and IP address %s for Phase I part 2\n", portnum, ipaddr);

    string msg = parse_seller_info("Agent2.txt");
    strcpy(buf, msg.c_str());
    sendto(udp_sockfd, buf, 1024, 0, (const struct sockaddr *) &udp_servaddr, sizeof(udp_servaddr));
    printf("<Agent2> has sent <%s> to <Buyer%d>\n", buf, buyer_name);
}

/* Phase I part 3 */
void receive_from_buyers(int numOfBuyers) {
    int newsockfd, pid;
    char buffer[256];
    int numbytes;
    struct hostent *he;
    memset(buffer, 0, 256);
    socklen_t clilen = sizeof(tcp_cliaddr);

    if ((he = gethostbyname("nunki.usc.edu")) == NULL) perror("gethostbyname");
    printf("<Agent2> has TCP port %d and IP address %s for Phase I part 3\n", Agent2_TCP_PORT, inet_ntoa(*(struct in_addr*)he->h_addr));

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
vector< vector <string> > calculate_bid() {
    vector< vector <string> > bids = parse_buyer_info("Agent2.txt");
    vector<string> res;
    res.push_back("NAK");
    res.push_back("NAK");
    vector<string> interested_buyers;
    int bidC = 0;
    int bidD = 0;
    for (int i = 0; i < 5; i++) {
        vector<string> bid = bids[i];
        if (bid[1] == "sellerC") {
            int curr_bid = atoi(bid[2].c_str());
            interested_buyers.push_back(bid[0]);  
            if (curr_bid > bidC) { // if current budget is larger than max budget
                res[0] = bid[0];  // update seller with this budget
                bidC = curr_bid;  // update max budget
            }
        }
        if (bid[1] == "sellerD") {
            int curr_bid = atoi(bid[2].c_str());
            interested_buyers.push_back(bid[0]);  
            if (curr_bid > bidD) { // if current budget is larger than max budget
                res[1] = bid[0];  // update seller with this budget
                bidD = curr_bid;  // update max budget
            }
        }
    }

    vector <vector <string> > inf;
    inf.push_back(res);
    inf.push_back(interested_buyers);

    return inf;
}

/* Phase I part 4 */
void send_final_result(int port, string name, string result) {
    char buf[1024];
    int numbyte;

    int tcp_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_sockfd == -1) perror("Socket");
    memset((void *) &tcp_servaddr, 0, sizeof(tcp_servaddr));
    tcp_servaddr.sin_family = AF_INET;
    tcp_servaddr.sin_port = htons(port);
    tcp_servaddr.sin_addr.s_addr = INADDR_ANY;

    if (-1 == connect(tcp_sockfd, (struct sockaddr *)&tcp_servaddr, sizeof(tcp_servaddr))) perror("Connect");
    strcpy(buf, result.c_str());
    send(tcp_sockfd, buf, 1024, 0);
    printf("<Agent2> has sent the result to <%s>\n", name.c_str());
    sleep(1);
}

void get_client_tcpip() {
    char ipaddr[INET_ADDRSTRLEN];
    memset(&tcp_cliaddr, 0, sizeof(tcp_cliaddr));
    getsockname(tcp_sockfd, (struct sockaddr *) &tcp_cliaddr, (socklen_t *) sizeof(tcp_cliaddr));
    inet_ntop(AF_INET, &tcp_cliaddr.sin_addr, ipaddr, sizeof(ipaddr));
    printf("<Agent2> has TCP port %d and IP address %s for Phase I part 4\n", ntohs(tcp_cliaddr.sin_port), ipaddr);
}

void get_client_udpip() {
    char ipaddr[INET_ADDRSTRLEN];
    memset(&udp_cliaddr, 0, sizeof(udp_cliaddr));
    getsockname(udp_sockfd, (struct sockaddr *) &udp_cliaddr, (socklen_t *) sizeof(udp_cliaddr));
    inet_ntop(AF_INET, &udp_cliaddr.sin_addr, ipaddr, sizeof(ipaddr));
    printf("<Agent2> has UDP port %d and IP address %s for Phase I part 2\n", ntohs(udp_cliaddr.sin_port), ipaddr);
}

int main(int argc, char *argv[]) {

    tcp_server(Agent2_TCP_PORT, 1);
    receive_from_sellers(2);
    create_udp_socket();
    get_client_udpip();
    send_to_buyer(21776, 1); // send to buyer1
    send_to_buyer(21876, 2); // send to buyer2
    send_to_buyer(21976, 3); // send to buyer3
    send_to_buyer(22076, 4); // send to buyer4
    send_to_buyer(22176, 5); // send to buyer5
    clear_seller_info("Agent2.txt");
    cout << "End of Phase I part 2 for <Agent2>" << endl;
    close(udp_sockfd);
    receive_from_buyers(5);
    vector< vector <string> > inf = calculate_bid();
    vector<string> to_sellers = inf[0]; // final results for sellers
    vector<string> to_buyers = inf[1];  // final results for buyers
    clear_seller_info("Agent2.txt");
    
    get_client_tcpip();
    map<string, string> lmao;
    send_final_result(3876, "sellerC", to_sellers[0]);
    lmao[to_sellers[0]] = "sellerC";
    send_final_result(3976, "sellerD", to_sellers[1]);
    lmao[to_sellers[1]] = "sellerD";
    map<string, int> all_buyers_port_num; // store buyer's static tcp port in a map
    all_buyers_port_num["Buyer1"] = 4276;
    all_buyers_port_num["Buyer2"] = 4376;
    all_buyers_port_num["Buyer3"] = 4476;
    all_buyers_port_num["Buyer4"] = 4576;
    all_buyers_port_num["Buyer5"] = 4676;

    /* Iterate through all to_buyers message, if the buyer want to buy house from the seller
       who is connected with Agent2, then send result to this buyer */
    for (int i = 0; i < to_buyers.size(); i++) {
        string msg = to_buyers[i];
        int buyer_port_num = all_buyers_port_num[msg];
        if (msg != to_sellers[0] and msg != to_sellers[1]) send_final_result(buyer_port_num, msg, "NAK");
        else send_final_result(buyer_port_num, msg, lmao[msg]);
    }

    cout << "End of Phase I part 4 for <Agent2>" << endl;


    return 0;
}
