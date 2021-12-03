#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sqlite3.h>
#include <sys/wait.h>

extern int errno;
const int port = 2022;
const char* IP = "192.168.1.231";

int main() {
    int socketDescriptor;
    struct sockaddr_in servInfo;

    if((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[client]Eroare socket()\n");
        return errno;
    }

    servInfo.sin_family = AF_INET;
    if(inet_pton(AF_INET, IP, &servInfo.sin_addr.s_addr) < 1)
    {
        perror("[client]Eroare inet_pton().\n");
        return errno;
    }
    servInfo.sin_port = htons(port);

    if(connect(socketDescriptor, (struct sockaddr*) &servInfo, sizeof (struct sockaddr)) == -1) {
        perror("[client]Eroare connect().\n");
        return errno;
    }

    close(socketDescriptor);
    return 0;
}