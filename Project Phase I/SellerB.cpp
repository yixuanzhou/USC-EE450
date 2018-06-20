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

#define PORT 3776  // 3400 + 376 (uscid:3827-1583-76)

using namespace std;

string readFile(string filename) {
    string res = "";
    string seller = filename.substr(0, 7);
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

    return seller+","+res.substr(0, res.length()-1);
}

int main(int argc, char *argv[]) {
	struct sockaddr_in servaddr;
	char buf[1024];
	int numbyte;
	char s[INET_ADDRSTRLEN];

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock == -1) perror("Socket");

	bzero((void *) &servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(4076);
	servaddr.sin_addr.s_addr = INADDR_ANY;

	struct sockaddr_in* pV4Addr = (struct sockaddr_in*) &servaddr;
	struct in_addr ipAddr = pV4Addr->sin_addr;
	char str[INET_ADDRSTRLEN];
	inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );

	printf("<SellerA> has TCP port %d and IP address %s for Phase I part 1", PORT, str);

	if (-1 == connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr))) perror("Connect");

	printf("<SellerA> is now connected to the <Agent1>");

	string data = readFile("sellerB.txt");
	//cout << data << endl;
	strcpy(buf, data.c_str());  
	
	if ((numbyte = send(sock, buf, 1024, 0)) > 0) {
		cout << "Sent" << endl;
	}

	if ((numbyte = recv(sock, buf, 1024, 0)) > 0) {
		cout << "Recv" << endl;
	}

	return 0;
}