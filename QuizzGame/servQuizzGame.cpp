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

#define PORT 2022

extern int errno;
const char* IP = "192.168.1.231";

int main() {
    struct sockaddr_in servInfo;
    struct sockaddr_in clInfo;
    int socketDescriptor;

    if((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[servInfo]Eroare socket().\n");
        return errno;
    }

    bzero (&servInfo, sizeof (servInfo));
    bzero (&clInfo, sizeof (clInfo));

    servInfo.sin_family = AF_INET;
    if(inet_pton(AF_INET, IP, &servInfo.sin_addr.s_addr) < 1)
	{
		perror("[server]Eroare inet_pton().\n");
		return errno;
	}
    servInfo.sin_port = htons(PORT);
    memset(&servInfo.sin_zero, 0, sizeof servInfo.sin_zero);

    if(bind(socketDescriptor, (struct sockaddr*) &servInfo, sizeof (struct sockaddr)) == -1) {
        perror("[server]Eroare bind().\n");
        return errno;
    }

    if(listen(socketDescriptor, 1) == -1) {
        perror("[server]Eroare listen().\n");
        return errno;
    }

    while(1) {
        int client;
        int length = sizeof(clInfo);

        client = accept(socketDescriptor, (struct sockaddr*) &clInfo, reinterpret_cast<socklen_t *>(&length));
        if(client < 0) {
            perror("[server]Eroare accept()\n");
            continue;
        }

        int pid;
        if((pid = fork()) == -1) {
            close(client);
            continue;
        }
        else if (pid > 0) {         //parent
            close(client);
            while(waitpid(-1, NULL, WNOHANG));
            continue;
        }
        else if (pid == 0) {        //child
            close(socketDescriptor);
            close(client);
            exit(0);
        }
    }
        

    return 0;
}
