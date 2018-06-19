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

#define PORT "3676"  // 3300 + 376 (uscid:3827 1583 76)

using namespace std;

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

int main(void) {
    struct sockaddr_in myaddr ,clientaddr;
    int sockid, newsockid, client_socket[3];
    char buf[1024];
    int numbyte;
    int max_clients = 3, sd, activity, valread;  
    int max_sd;

    char *message = "ECHO Daemon v1.0 \r\n";  

    for (int i = 0; i < max_clients; i++) client_socket[i] = 0;

    sockid = socket(AF_INET, SOCK_STREAM, 0);
    memset(&myaddr, '0', sizeof(myaddr));

    fd_set readfds;
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(3300);
    myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (sockid == -1) perror("socket");
    
    socklen_t addrlen = sizeof(myaddr);
    //int addrlen = sizeof(myaddr);
    if (bind(sockid, (struct sockaddr *)&myaddr, addrlen) < 0) perror("bind");

    if (listen(sockid, 3) == -1) perror("listen");

    int pid;
    int counter = 0;

    while (1) {
        newsockid = accept(sockid, (struct sockaddr *)&clientaddr, &addrlen);

        if ((pid = fork()) == -1) {
            close(newsockid);
            continue;
        }
        else if (pid > 0) {
            close(newsockid);
            counter++;
            printf("here2\n");
            continue;
        }
        else if (pid == 0) {
            char buf[100];
            counter++;
            printf("here 1\n");
            snprintf(buf, sizeof buf, "hi %d", counter);
            send(newsockid, buf, strlen(buf), 0);
            close(newsockid);
            break;
        }
    }

    /* UDP */
    // Creating socket file descriptor
    /*sockid = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&myaddr, '0', sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(21776);
    myaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

     
    if ((numbyte = sendto(sockid, buf, 1024, 0, (const struct sockaddr *) &myaddr, sizeof(myaddr))) > 0) {
        cout << "Info sent to buyers" << endl;
    }
     /*    
    if n = recvfrom(sockfd, (char *)buffer, MAXLINE, 
                MSG_WAITALL, (struct sockaddr *) &servaddr,
                &len);
    buffer[n] = '\0';*/
 /*
    close(sockid);*/
    return 0;
}