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

#define TCP_PORT 4076

using namespace std;

// TWO VECTORS TO STORE SELLER'S INFORMAITON
string sellerA_INFO[2];
string sellerB_INFO[2];

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
 
void doprocessing(int sock) {
    int numbytes;
    char buffer[256];
    bzero(buffer,256);
    numbytes = read(sock, buffer, 255);

    if (numbytes < 0) perror("ERROR reading from socket");
    vector<string> msg = parseMsg(buffer);

    if (msg[0] == "sellerA") {
        cout << "sellerA received" << endl;
        sellerA_INFO[0] = msg[2];
        sellerA_INFO[1] = msg[4];
    }

    if (msg[0] == "sellerB") {
        cout << "sellerB received" << endl;
        sellerB_INFO[0] = msg[2];
        sellerB_INFO[1] = msg[4];
    }

    printf("Here is the message: %s\n", buffer);
}

// Create and handle tcp socket connections
void tcp_server(int port) {

    tcp_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_sockfd < 0) perror("ERROR opening socket");

    bzero((char *) &tcp_servaddr, sizeof(tcp_servaddr)); // make sure the struct is empty

    tcp_servaddr.sin_family = AF_INET;
    tcp_servaddr.sin_addr.s_addr = INADDR_ANY;
    tcp_servaddr.sin_port = htons(TCP_PORT);

    if (bind(tcp_sockfd, (struct sockaddr *) &tcp_servaddr, sizeof(tcp_servaddr)) < 0) perror("ERROR on binding");

    if (listen(tcp_sockfd, 5) < 0) perror("ERROR on listening");
    
}

void receive_from_sellers(int numOfClients) {
    int newsockfd;
    char buffer[256];
    int pid;

    socklen_t clilen = sizeof(tcp_cliaddr);

    for(int ct = 0; ct < numOfClients; ct++) {
        newsockfd = accept(tcp_sockfd, (struct sockaddr *) &tcp_cliaddr, &clilen);
        if (newsockfd < 0) perror("ERROR on accept");

        pid = fork();
        if (pid < 0) perror("ERROR on fork");
        if (pid == 0) {
            close(tcp_sockfd);
            doprocessing(newsockfd);
            exit(0);
        } else close(newsockfd);
    }
}

void create_udp_socket() {
    udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd == -1) perror("socket");    
}

void send_to_buyer(int port) {
    char buf[1024];

    memset(&udp_servaddr, 0, sizeof(udp_servaddr));

    udp_servaddr.sin_family = AF_INET;
    udp_servaddr.sin_port = htons(port);
    udp_servaddr.sin_addr.s_addr = INADDR_ANY;
    socklen_t addrlen = sizeof(udp_servaddr);
     
    sendto(udp_sockfd, buf, 1024, 0, (const struct sockaddr *) &udp_servaddr, sizeof(udp_servaddr));
}
 
int main(int argc, char *argv[]) {

    tcp_server(TCP_PORT);
    receive_from_sellers(2);
    create_udp_socket();
    send_to_buyer(21776);
    send_to_buyer(21876);

    return 0;
}
