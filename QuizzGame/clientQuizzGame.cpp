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
bool timeExpired = false;

struct Question {
    char question[100];
    char ans1[50], ans2[50], ans3[50], ans4[50];
};

void sigHandler(int sign)
{
    if(sign == SIGALRM) {
        timeExpired = true;
        printf(" ");
    }
}


Question* recvQuestion(const int& sd) 
{
    int len, nb;
    Question* quiz = (Question*) malloc(sizeof(Question));

    // Question
    nb = recv(sd, &len, sizeof(int), 0);
    if (nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }
    
    nb = recv(sd, quiz->question, len + 1, 0);
    if (nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }

    // Answer A
    nb = recv(sd, &len, sizeof(int), 0);
    if (nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }
    
    nb = recv(sd, quiz->ans1, len + 1, 0);
    if (nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }

    // Answer B
    nb = recv(sd, &len, sizeof(int), 0);
    if (nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }
    
    nb = recv(sd, quiz->ans2, len + 1, 0);
    if (nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }

    // Answer C
    nb = recv(sd, &len, sizeof(int), 0);
    if (nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }
    
    nb = recv(sd, quiz->ans3, len + 1, 0);
    if (nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }

    // Answer D
    nb = recv(sd, &len, sizeof(int), 0);
    if (nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }
    
    nb = recv(sd, quiz->ans4, len + 1, 0);
    if (nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }

    return quiz;
}

void printQuestion(const Question& quiz)
{
    int row, col;
    getmaxyx(stdscr, row, col);
    mvprintw(1.5 * row / 8, (col - strlen(quiz.question)) / 2, "Q: %s", quiz.question);
    mvprintw(3.5 * row / 8, (col - strlen(quiz.ans1)) / 6, "A. %s", quiz.ans1);
    mvprintw(3.5 * row / 8, 4 * (col - strlen(quiz.ans2)) / 6, "B. %s", quiz.ans2);
    mvprintw(5.5 * row / 8, (col - strlen(quiz.ans3)) / 6, "C. %s", quiz.ans3);
    mvprintw(5.5 * row / 8, 4 * (col - strlen(quiz.ans4)) / 6, "D. %s", quiz.ans4);
    refresh();
}

int main() {

    if (signal(SIGALRM, sigHandler) == SIG_ERR) {
        perror("[client]Eroare signal.\n");
        return errno;
    }

    int socketDescriptor;
    struct sockaddr_in servInfo;
    
    std::cout << "Connecting to server...\n";

    if ((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[client]Eroare socket()\n");
        return errno;
    }

    servInfo.sin_family = AF_INET;
    if (inet_pton(AF_INET, IP, &servInfo.sin_addr.s_addr) < 1) {
        perror("[client]Eroare inet_pton().\n");
        return errno;
    }
    servInfo.sin_port = htons(PORT);

    if (connect(socketDescriptor, (struct sockaddr*) &servInfo, sizeof (struct sockaddr)) == -1) {
        perror("[client]Eroare connect().\n");
        return errno;
    }

    std::cout << "Connection established.\n";

    int nb;
    bool accepted;
    nb = recv(socketDescriptor, &accepted, sizeof(bool), 0);
    if (nb != 1) {
        perror("[client]Eroare recv()\n");
        close(socketDescriptor);
        return errno;
    }

    if (!accepted) {
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
        if (nb <= 0) {
            perror("[client]Eroare send()\n");
            close(socketDescriptor);
            return errno;
        }
        nb = send(socketDescriptor, username.c_str(), len + 1, 0);
        if (nb <= 0) {
            perror("[client]Eroare send()\n");
            close(socketDescriptor);
            return errno;
        }
        nb = recv(socketDescriptor, &accepted, sizeof(bool), 0);
        if (nb != 1) {
            perror("[client]Eroare recv()\n");
            close(socketDescriptor);
            return errno;
        }
        if (!accepted) {
            std::cout << "Username already in use. \n";
        }
    } while (!accepted);

    initscr();
    cbreak();
    noecho();
    int row, col;
    getmaxyx(stdscr, row, col);
    char msg1[] = "Press any key to start the game.";
    char msg2[] = "Time expired!";
    char msg3[] = "Waiting for other players.";
    
    //std::cout << "Press any key to start the game.\n";

    mvprintw(row / 2, (col - strlen(msg1)) / 2, "%s", msg1);

    char key = 0;
    key = getch();
    nb = send(socketDescriptor, &key, 1, 0);
    if (nb <= 0) {
        perror("[client]Eroare send()\n");
        close(socketDescriptor);
        return errno;
    }
    clear();
    refresh();

    Question* quiz;
    quiz = recvQuestion(socketDescriptor);

    printQuestion(*quiz);

    int pipeD[2];
    if (pipe(pipeD) < 0) {
        perror("Eroare pipe\n");
        return 3;
    }

    pid_t pid;
    pid = fork();
    if (pid < 0) {
        exit(2);
    } else if (pid == 0) {
        close(socketDescriptor);
        close(pipeD[0]);
        free(quiz);
        int n;
        key = getch();
        n = write(pipeD[1], &key, sizeof(char));
        if (n <= 0) {
            perror("[client]Eroare write()\n");
            close(pipeD[1]);
            return errno;
        }
        close(pipeD[1]);
        return 0;
        // n = recv(socketDescriptor, &expired, sizeof(bool), 0);
        // if(n < 0) {
        //     //perror("[client]Eroare recv()\n");
        //     close(socketDescriptor);
        //     return errno;
        // }
        // if(expired) {
        //     kill(getppid(), SIGALRM);
        //     exit(0);
        // }
    } else {
        close(pipeD[1]);
        bool timeout;
        n = recv(socketDescriptor, &timeout, sizeof(bool), 0);
        if(n < 0) {
            //perror("[client]Eroare recv()\n");
            close(socketDescriptor);
            close(pipeD[0]);
            return errno;
        }
        if(timeout) {
            kill(pid, SIGTERM);
            mvprintw(row / 2, (col - strlen(msg2)) / 2, "%s", msg2);
        } else {
            nb = read(pipeD[0], &key, sizeof(char));
            if (n <= 0) {
                perror("[client]Eroare read()\n");
                close(pipeD[0]);
                close(socketDescriptor);
                free(quiz);
                return errno;
            }
            nb = send(socketDescriptor, &key, sizeof(char), 0);
            if (nb <= 0) {
                perror("[client]Eroare send()\n");
                close(socketDescriptor);
                return errno;
            }
        }
        clear();
        refresh();
        mvprintw(row / 2, (col - strlen(msg3)) / 2, "%s", msg3);
        refresh();
        free(quiz);
        close(socketDescriptor);
        while(1);
        return 0;
    }
    
}