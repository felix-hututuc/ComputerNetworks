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
#include <iostream>
#include <string>
#include <curses.h>

extern int errno;
const int PORT = 2022;
const char* IP = "192.168.1.231";

int main() {
    int socketDescriptor;
    struct sockaddr_in servInfo;
    
    std::cout << "Connecting to server...\n";

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
    servInfo.sin_port = htons(PORT);

    if(connect(socketDescriptor, (struct sockaddr*) &servInfo, sizeof (struct sockaddr)) == -1) {
        perror("[client]Eroare connect().\n");
        return errno;
    }

    std::cout << "Connection established.\n";

    int nb;
    bool accepted;
    nb = recv(socketDescriptor, &accepted, sizeof(bool), 0);
    if(nb != 1) {
        perror("[client]Eroare recv()\n");
        close(socketDescriptor);
        return errno;
    }

    if(!accepted) {
        std::cout << "The game is already in progress. Please try again later.\n";
        close(socketDescriptor);
        return 0;
    }

    std::string username;
    do {
        std::cout << "Insert a username: ";
        std::cin >> username;
        int len = username.size();
        nb = send(socketDescriptor, &len, sizeof(int), 0);
        if(nb <= 0) {
            perror("[client]Eroare send()\n");
            close(socketDescriptor);
            return errno;
        }
        nb = send(socketDescriptor, username.c_str(), len + 1, 0);
        if(nb <= 0) {
            perror("[client]Eroare send()\n");
            close(socketDescriptor);
            return errno;
        }
        nb = recv(socketDescriptor, &accepted, sizeof(bool), 0);
        if(nb != 1) {
            perror("[client]Eroare recv()\n");
            close(socketDescriptor);
            return errno;
        }
        if(!accepted) {
            std::cout << "Username already in use. \n";
        }
    }while(!accepted);
    //std::cout << "Press any key to start the game.\n";

    // char key = getch();
    // nb = send(socketDescriptor, &key, 1, 0);
    // if(nb <= 0) {
    //     perror("[client]Eroare send()\n");
    //     close(socketDescriptor);
    //     return errno;
    // }

    close(socketDescriptor);
    return 0;
}