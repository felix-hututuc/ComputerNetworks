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
#include <curses.h>
#include <pthread.h>
#include <vector>
#include <map>
#include <signal.h>
#include "Player.h"

#define PORT 2022

extern int errno;
const char* IP = "192.168.1.231";
bool acceptPlayers = true, openServer = true;
int numberOfPlayers = 0;
std::vector<Player> players;
pthread_mutex_t mutexPlayerVec;

void sigHandler(int sign)
{
    if(sign == SIGINT) {
        openServer = false;
        printf("Exiting server\n");
    }
}

bool isPlayer(const std::string& username) 
{
    for(auto p : players) {
        if(p.getUsername() == username)
            return true;
    }
    return false;
}   

void* playerRoutine(void* args) {
    int client = *(int*) args;
    int len, nb;
    bool loggedIn = 0;
    std::string strUsername;
    do {
        nb = recv(client, &len, sizeof(int), 0);
        if(nb < 0) {
            perror("[server]Eroare recv()\n");
            close(client);
            exit(0);
        }
        char* username = new char[len + 1];
        nb = recv(client, username, len + 1, 0);
        if(nb != len + 1) {
            perror("[server]Eroare recv()\n");
            close(client);
            exit(0);
        }
        strUsername = username;
        delete(username);

        pthread_mutex_lock(&mutexPlayerVec);
        //search for username
        if(!isPlayer(strUsername)) {              //username not found
            loggedIn = 1;
        }
        pthread_mutex_unlock(&mutexPlayerVec);
        nb = send(client, &loggedIn, sizeof(bool), 0);
        if(nb < 0) {
            perror("[server]Eroare send()\n");
            close(client);
            exit(0);
        }
    }while(!loggedIn);

    Player player(strUsername);
    int indexInVec;

    pthread_mutex_lock(&mutexPlayerVec);
    indexInVec = players.size();
    players.push_back(player);
    numberOfPlayers++;
    pthread_mutex_unlock(&mutexPlayerVec);

    close(client);
    free(args);
    return NULL;
}

int main(int argc, char* argv[]) {

    pid_t pid;
    if((pid = fork()) < 0) {
        perror("[server]Eroare fork\n");
        return errno;
    }
    if(pid == 0) {
        while(1) {
            std::string command;
            std::cin >> command;
            if(command == "quit") {
                kill(getppid(), SIGINT);
                exit(0);
            }
        }
    } else { 
        pthread_t* playerThreads;
        pthread_attr_t detachedThreadAttr;
        pthread_attr_init(&detachedThreadAttr);
        pthread_attr_setdetachstate(&detachedThreadAttr, PTHREAD_CREATE_DETACHED);
        pthread_mutex_init(&mutexPlayerVec, NULL);

        struct sigaction newHandler;
        struct sigaction oldHandler;
        memset(&newHandler, 0, sizeof(newHandler));
        newHandler.sa_handler = sigHandler;
        sigemptyset(&newHandler.sa_mask);
        newHandler.sa_flags = 0;
        if(sigaction(SIGINT, &newHandler, &oldHandler) < 0) {
            perror("[server]Eroare signal.\n");
            return errno;
        }

        // if(signal(SIGINT, sigHandler) == SIG_ERR) {
        //     perror("[server]Eroare signal.\n");
        //     return errno;
        // }

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

        while(openServer) {
            int client;
            int length = sizeof(clInfo);
            int nb;

            client = accept(socketDescriptor, (struct sockaddr*) &clInfo, (socklen_t *) &length);
            if(client < 0) {
                perror("[server]Eroare accept()\n");
                continue;
            }
            if(!openServer) {
                std::cout << "HERE\n";
                break;
            }
            nb = send(client, &acceptPlayers, sizeof(bool), 0);
            if(nb <= 0) {
                perror("[server]Eroare send()\n");
                close(client);
                return errno;
            }
            if(!acceptPlayers) {
                close(client);
                continue;
            } else {
                playerThreads[numberOfPlayers] = (pthread_t)malloc(sizeof(pthread_t));
                int* sd = (int*) malloc(sizeof(int));
                *sd = client;
                if(pthread_create(&playerThreads[numberOfPlayers], NULL, &playerRoutine, sd) != 0) {
                    perror("[server]Failed to create thread.\n");
                    return errno;
                }
                continue;
            }
        }

        pthread_attr_destroy(&detachedThreadAttr);
        pthread_mutex_destroy(&mutexPlayerVec);

        for(int i = 0; i < numberOfPlayers; i++) {
            if(pthread_join(playerThreads[i], NULL) < 0) {
                perror("[server]Eroare join threads\n");
                return errno;
            }
        }

        return 0;
    }
}
