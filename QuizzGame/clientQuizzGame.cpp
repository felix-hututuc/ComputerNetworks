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

struct Question {
    char question[100];
    char ans1[50], ans2[50], ans3[50], ans4[50];
};

Question* recvQuestion(const int& sd) 
{
    int len, nb;
    Question* quiz = (Question*) malloc(sizeof(Question));

    // Question
    nb = recv(sd, &len, sizeof(int), 0);
    if(nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }
    
    nb = recv(sd, quiz->question, len + 1, 0);
    if(nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }

    // Answer A
    nb = recv(sd, &len, sizeof(int), 0);
    if(nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }
    
    nb = recv(sd, quiz->ans1, len + 1, 0);
    if(nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }

    // Answer B
    nb = recv(sd, &len, sizeof(int), 0);
    if(nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }
    
    nb = recv(sd, quiz->ans2, len + 1, 0);
    if(nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }

    // Answer C
    nb = recv(sd, &len, sizeof(int), 0);
    if(nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }
    
    nb = recv(sd, quiz->ans3, len + 1, 0);
    if(nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }

    // Answer D
    nb = recv(sd, &len, sizeof(int), 0);
    if(nb < 0) {
        perror("[server]Eroare recv()\n");
        close(sd);
        exit(-1);
    }
    
    nb = recv(sd, quiz->ans4, len + 1, 0);
    if(nb < 0) {
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

    initscr();
    cbreak();
    noecho();
    int row, col;
    getmaxyx(stdscr, row, col);
    char msg[] = "Press any key to start the game.";
    
    //std::cout << "Press any key to start the game.\n";

    mvprintw(row / 2, (col - strlen(msg)) / 2, "%s", msg);

    char key = 0;
    key = getch();
    nb = send(socketDescriptor, &key, 1, 0);
    if(nb <= 0) {
        perror("[client]Eroare send()\n");
        close(socketDescriptor);
        return errno;
    }
    clear();
    refresh();

    Question* quiz;
    quiz = recvQuestion(socketDescriptor);

    printQuestion(*quiz);
    
    free(quiz);
    close(socketDescriptor);
    while(1);
    return 0;
}