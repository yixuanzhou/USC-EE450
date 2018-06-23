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
#include <arpa/inet.h>

#define SellerA_TCP_PORT 3676
#define HOSTNAME "nunki.usc.edu"

using namespace std;

int tcp_sockfd;
struct sockaddr_in tcp_servaddr, tcp_cliaddr;

/* Read local txt file */
/* Reused from lab1 */
string readFile(string filename) {
    string res = "";
    ifstream infile(filename.c_str());
    string line;
    while (getline(infile, line)) {
        stringstream iss(line);
        string s;
        while (iss.good()) {
            getline(iss, s, ':');
            res += (s+",");
        }
    }

    return "sellerA,"+res.substr(0, res.length()-1);
}

/* Phase I Part 4 */
void create_tcp_server() {
    struct hostent *he;
    char ipaddr[INET_ADDRSTRLEN];

    tcp_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (tcp_sockfd < 0) perror("ERROR opening socket");

    memset((char *) &tcp_servaddr, 0, sizeof(tcp_servaddr)); // make sure the struct is empty

    tcp_servaddr.sin_family = AF_INET;
    tcp_servaddr.sin_addr.s_addr = INADDR_ANY;
    tcp_servaddr.sin_port = htons(SellerA_TCP_PORT);

    he = gethostbyname("nunki.usc.edu");

    if (bind(tcp_sockfd, (struct sockaddr *) &tcp_servaddr, sizeof(tcp_servaddr)) < 0) perror("ERROR on binding");
    if (listen(tcp_sockfd, 5) < 0) perror("ERROR on listening");

    cout << "<SellerA> has TCP port " << SellerA_TCP_PORT << " and IP address " << inet_ntoa(*(struct in_addr*)he->h_addr) << " for Phase I part 4" << endl;
}

/* Phase I Part 1 */
void create_tcp_client() {
	char ipaddr[INET_ADDRSTRLEN];
    struct hostent *he;

    if ((he = gethostbyname("localhost")) == NULL) perror("gethostbyname");

	tcp_sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (tcp_sockfd == -1) perror("Socket");

	memset((void *) &tcp_servaddr, 0, sizeof(tcp_servaddr));
	tcp_servaddr.sin_family = AF_INET;
	tcp_servaddr.sin_port = htons(4076);
	tcp_servaddr.sin_addr = *((struct in_addr *)he->h_addr);
    //memset(&(tcp_servaddr.sin_zero), '\0', 8); // zero the rest of the struct

	//strcpy(ipaddr, inet_ntoa(tcp_servaddr.sin_addr));
    //cout << "<SellerA> has TCP port " << SellerA_TCP_PORT << " and IP address " << ipaddr << " for Phase I part 1" << endl;
}

/* Phase I Part 1 */
void send_to_agent() {
	char buf[1024];
	int numbyte;
    char ipaddr[INET_ADDRSTRLEN];

	if (-1 == connect(tcp_sockfd, (struct sockaddr *)&tcp_servaddr, sizeof(tcp_servaddr))) perror("Connect");
    
    getsockname(tcp_sockfd, (struct sockaddr *) &tcp_cliaddr, (socklen_t *)sizeof(tcp_cliaddr));
    inet_ntop(AF_INET, &tcp_cliaddr.sin_addr, ipaddr, sizeof(ipaddr));
    printf("<SellerA> has TCP port %d and IP address %s for Phase I part 1\n", ntohs(tcp_cliaddr.sin_port), ipaddr);
	cout << "<SellerA> is now connected to the <Agent1>" << endl;

	string data = readFile("seller/sellerA.txt");
	strcpy(buf, data.c_str());  
	
	if ((numbyte = send(tcp_sockfd, buf, 1024, 0)) > 0) {
		cout << "<SellerA> has sent <" << buf << "> to the agent" << endl;
	}
	cout << "End of Phase I part 1 for <SellerA>" << endl;
}

/* Receive final result from agent */
void receive_from_agent() {
    int newsockfd;
    char buffer[256];
    int numbytes;
    socklen_t clilen = sizeof(tcp_cliaddr);
    newsockfd = accept(tcp_sockfd, (struct sockaddr *) &tcp_cliaddr, &clilen);
    read(newsockfd, buffer, 255);
    string buyer_name = string(buffer);
    if (buyer_name != "NAK") printf("<%s> will buy my house\n", buffer);
    else cout << "NAK" << endl; // if no one want to buy my house
    sleep(1);
    cout << "End of Phase I part 4 for <SellerA>" << endl;
}

int main(int argc, char *argv[]) {
	create_tcp_client();
	send_to_agent();
	create_tcp_server();
    receive_from_agent();

	return 0;
}
