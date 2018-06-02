#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>

#define FIFO_FILE "BELLMANFORD"

int main() {
    int fd;
    char readbuf[80];
    char end[10];
    int to_end;
    int read_bytes;

    /* Create the FIFO if it does not exist */
    mknod(FIFO_FILE, S_IFIFO|0640, 0);
    strcpy(end, "end");
    while (true) {
        fd = open(FIFO_FILE, O_RDONLY);
        read_bytes = read(fd, readbuf, sizeof(readbuf));
        readbuf[read_bytes] = '\0';
        if (read_bytes > 0) {
            char cf[20];
            std::string filename = "N" + std::string(readbuf) + ".csv";
            strcpy(cf, filename.c_str());
            printf("Copy that!");
            execl("BellmanFord", "BellmanFord", cf, (char *) 0);
        }
        to_end = strcmp(readbuf, end);
        if (to_end == 0) {
            close(fd);
            break;
        }
    }
    return 0;
}