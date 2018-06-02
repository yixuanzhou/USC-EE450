#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <string>

#define FIFO_FILE "BELLMANFORD"

int main() {
    int fd;
    int stringlen;
    int end_process;

    char readbuf[80];
    char end_str[5];
    fd = open(FIFO_FILE, O_CREAT|O_WRONLY);

    strcpy(end_str, "end");

    while (true) {
        printf("Enter the number of nodes you want to create: ");
        fgets(readbuf, sizeof(readbuf), stdin);
        stringlen = strlen(readbuf);
        readbuf[stringlen - 1] = '\0';
        end_process = strcmp(readbuf, end_str);

        if (end_process != 0) {
            write(fd, readbuf, strlen(readbuf));
            execl("GraphGen", "GraphGen", readbuf, (char *) 0);
        } else {
            write(fd, readbuf, strlen(readbuf));
            close(fd);
            break;
        }
    }
    return 0;
}