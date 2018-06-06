/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include <iostream>
#include <random>

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 

using namespace std;
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

string randTransID () {
	default_random_engine generator;
  	uniform_int_distribution<int> distribution(0,1);
    int num = 0;

    for (int i = 0; i < 8; i++) {
        int d = distribution(generator);
        cout << d;
        num += (d * 2 ^ i);
    }
    return to_string(num);
}

int main(int argc, char *argv[])
{
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

	string id;

	id = randTransID();
	cout << id << endl;
    strcpy(buf, id.c_str());

    /* DISCOVER PHASE */
    if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) > 0) {
        cout << "Request sent, with Transaction ID: " << buf << endl;
    }

    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) > 0) {
    	cout << "IP address received: " << buf << endl;
    }

    string ipad = buf;

    /* REQUEST PHASE */
    id = randTransID();
	cout << id << endl;
    strcpy(buf, id.c_str());

    if ((numbytes = send(sockfd, buf, MAXDATASIZE-1, 0)) > 0) {
        cout << "Ok, I want this: " << ipad << ", with transaction ID: " << buf << endl;
    }

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	buf[numbytes] = '\0';
	close(sockfd);

	return 0;
}

