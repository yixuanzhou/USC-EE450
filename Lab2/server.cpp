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
#include <sstream>

#define PORT "3676"  // 3300 + 376 (uscid:3827 1583 76)
#define BACKLOG 10	 // how many pending connections queue will hold
#define MAXDATASIZE 1024 // max number of bytes we can get at once 

using namespace std;

void sigchld_handler(int s) {
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


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

string randomAddr (string transID) {
	srand(time(NULL));
    string addr = "";
    int curr = 0;

    for (int i = 0; i < 24; i++) {
        int r = rand() % 2;
        curr += (r * pow(2, (i % 8)));
        if (i % 8 == 7) {
            addr = toString(curr) + "." + addr;
            curr = 0;
        }
    }

    return addr + transID;
}

int main(void) {
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	int numbytes;
	char buf[1024];


	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);
		string addr;
		string id;

		/* RECEIVED REQUEST FROM CLIENT */
		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) > 0) {
		    cout << "Message Received, with Transaction ID: " << buf << endl;
		    sleep(3);    
		}

		/* OFFER PHASE */
		string recvID = string(buf);
		addr = randomAddr(recvID);
		id = randTransID();
		string msg = addr + "#" + id;
	   	strcpy(buf, msg.c_str());
		if ((numbytes = send(new_fd, buf, MAXDATASIZE-1, 0)) > 0) {			
			cout << "I can offer you with IP address: " << addr << ", with Transaction ID: " << id << endl;
		}		

		/* ACKNOWLEDGE PHASE */
		if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) > 0) {
			cout << "Received acknowledgment, with transaction ID: " << buf << endl;
			sleep(3);
		}

	    id = randTransID();
		strcpy(buf, id.c_str());
	    if ((numbytes = send(new_fd, buf, MAXDATASIZE-1, 0)) > 0) {
	    	cout << "Now you can use this address: " << addr << ", with Transaction ID: " << buf << endl;
		}		
	}

	return 0;
}
