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
#include <arpa/inet.h>

#define PORT "3676"  // 3300 + 376 (uscid:3827 1583 76)
#define MAXDATASIZE 1024 // max number of bytes we can get at once 

using namespace std;
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

string toString (int num) {
    stringstream ss;
    ss << num;
    return ss.str();
}

int pow(double x, int n) {
    if (n == 0) return 1;
    if (n == 1) return x;
    if (n < 0) {
        x = 1/x;
        return n % 2 == 0 ? pow(x*x, -(n/2)) : x * pow(x*x, -((n+1)/2));
    }
    return n % 2 == 0 ? pow(x*x, n/2) : x * pow(x*x, (n-1)/2);
}

string randTransID () {
	srand(time(NULL));

    int num = 0;
    for (int i = 0; i < 8; i++) {
        int d = rand() % 2;
        num += (d * pow(2, i));
    }
    return toString(num);
}

int main(int argc, char *argv[]) {
	int sockfd, numbytes;  
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	/* DISCOVER PHASE */
	string id;
	id = randTransID();
    strcpy(buf, id.c_str());    
    if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) > 0) {    	
        cout << "Broadcast: \"Can someone give me an address?\", with Transaction ID: " << buf << endl;
    }

    string recvAddr;
    string recvID;
    /* RECEIVED IP ADDRESS FROM DHCP SERVER */
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) > 0) {
    	string msg = string(buf);
    	int idx = msg.find('#');
    	recvAddr = msg.substr(0, idx);
    	recvID = msg.substr(idx+1);
    	cout << "Offer received, IP address: " << recvAddr << ", with transaction ID: " << recvID << endl;
  	    sleep(3);
    }

    /* REQUEST PHASE */
    id = randTransID();
    strcpy(buf, id.c_str());
    if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) > 0) {
        cout << "Ok, I want to take this offer: " << recvAddr << ", with transaction ID: " << buf << endl;
    }

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) > 0) {
	    cout << "Completed. Now using address: " << recvAddr << endl;
	}

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	buf[numbytes] = '\0';
	close(sockfd);

	return 0;
}
