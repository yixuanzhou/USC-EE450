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
    string msg = parse_seller_info("Agent2.txt");
    strcpy(buf, msg.c_str());
    clear_seller_info("Agent2.txt");
    sendto(udp_sockfd, buf, 1024, 0, (const struct sockaddr *) &udp_servaddr, sizeof(udp_servaddr));
    printf("<Agent2> has sent %s to <Buyer%d>\n", buf, buyer_name);
}

/* Phase I part 3 */
vector<pair<string, string>> receive_from_buyers(int numOfBuyers) {
    int newsockfd, pid;
    char buffer[256];
    int numbytes;
    bzero(buffer, 256);
    vector<pair<string, string>> res;
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
            string response = string(buffer);
            if (response == "NAK") continue;
            vector<string> msg = parseMsg(response);
            printf("<Agent2> receives the offer from <%s>\n", msg[0].c_str());  
            pair<string, string> bingo = make_pair(msg[0], msg[1]);
            res.push_back(bingo);
            exit(0);
        } else close(newsockfd);
    }
    sleep(1);
    cout << "End of Phase I part 3 for <Agent2>" << endl;
    return res;
}



void tcp_client(int port) {
    struct sockaddr_in servaddr;

    char ipaddr[INET_ADDRSTRLEN];

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == -1) perror("Socket");

    bzero((void *) &servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    strcpy(ipaddr, inet_ntoa(udp_servaddr.sin_addr));
    printf("<Agent2> has TCP port %d and IP address %s for Phase I part 1\n", Agent2_TCP_PORT, ipaddr);
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
    cout << "End of Phase I part 2 for <Agent2>" << endl;
    close(udp_sockfd);
    //tcp_server(Agent1_TCP_PORT, 3);
    vector<pair<string, string>> res;
    res = receive_from_buyers(1);
    
  

    return 0;
}
