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
    char ans1[100], ans2[100], ans3[100], ans4[100];
};

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
    clear();
    int row, col;
    // getmaxyx(stdscr, row, col);
    // mvprintw(1.5 * row / 8, (col - strlen(quiz.question)) / 2, "Q: %s", quiz.question);
    // mvprintw(3.5 * row / 8, (col - strlen(quiz.ans1)) / 6, "A. %s", quiz.ans1);
    // mvprintw(3.5 * row / 8, 4 * (col - strlen(quiz.ans2)) / 6, "B. %s", quiz.ans2);
    // mvprintw(5.5 * row / 8, (col - strlen(quiz.ans3)) / 6, "C. %s", quiz.ans3);
    // mvprintw(5.5 * row / 8, 4 * (col - strlen(quiz.ans4)) / 6, "D. %s", quiz.ans4);
    // refresh();
    std::cout << "Q: " << quiz.question << "\n";
    std::cout << "A: " << quiz.ans1 << "\n";
    std::cout << "B: " << quiz.ans2 << "\n";
    std::cout << "C: " << quiz.ans3 << "\n";
    std::cout << "D: " << quiz.ans4 << "\n";

}

int main() {
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

    // initscr();
    // cbreak();
    // noecho();
    int row, col;
    getmaxyx(stdscr, row, col);
    char msg1[] = "Press any key to start the game.";
    char msg2[] = "Time expired!";
    char msg3[] = "Waiting for other players.";
    
    std::cout << "Press any key to start the game.\n";

    //mvprintw(row / 2, (col - strlen(msg1)) / 2, "%s", msg1);

    char key = 0;
    //key = getch();
    std::cin >> key;
    nb = send(socketDescriptor, &key, 1, 0);
    if (nb <= 0) {
        perror("[client]Eroare send()\n");
        close(socketDescriptor);
        return errno;
    }
    // clear();
    // refresh();
    bool finished;
    do {
        Question* quiz;
        quiz = recvQuestion(socketDescriptor);
        system("clear");

        printQuestion(*quiz);

        pid_t pid;
        pid = fork();
        if (pid < 0) {
            exit(2);
        } else if (pid == 0) {
            free(quiz);
            int n;
            // key = getch();
            std::cin >> key;
            n = send(socketDescriptor, &key, sizeof(char), 0);
            if (n <= 0) {
                perror("[client]Eroare send()\n");
                close(socketDescriptor);
                return errno;
            }
            close(socketDescriptor);
            return 0;
        }
        bool answered;
        nb = recv(socketDescriptor, &answered, sizeof(bool), 0);
        if (nb < 0) {
            close(socketDescriptor);
            return errno;
        }
        if (!answered) {
            kill(pid, SIGTERM);
            // clear();
            // refresh();
            system("clear");
            // mvprintw(row / 2, (col - strlen(msg2)) / 2, "%s", msg2);
            // refresh();
            printf("%s\n", msg2);
            sleep(2);
        } 
        // clear();
        // mvprintw(row / 2, (col - strlen(msg3)) / 2, "%s", msg3);
        // refresh();
        printf("%s\n", msg3);
        free(quiz);
        nb = recv(socketDescriptor, &finished, sizeof(bool), 0);
        if (nb < 0) {
            close(socketDescriptor);
            return errno;
        }
        // refresh();
    }while (!finished);

    system("clear");

    std::cout << "Game ended!\n";

    int nrOfWinners, maxScore;
    nb = recv(socketDescriptor, &nrOfWinners, sizeof(int), 0);
    if (nb < 0) {
        close(socketDescriptor);
        return errno;
    }
    nb = recv(socketDescriptor, &maxScore, sizeof(int), 0);
    if (nb < 0) {
        close(socketDescriptor);
        return errno;
    }
    if (nrOfWinners > 1) {
        std::cout << "The winners are : \n";
    } else {
        std::cout << "The winner is : \n";
    }
    int len;
    char wUsername[50];
    for (int i = 0; i < nrOfWinners; i++) {
        nb = recv(socketDescriptor, &len, sizeof(int), 0);
        if (nb < 0) {
            perror("[server]Eroare send()3\n");
            close(socketDescriptor);
            exit(1);
        }
        nb = recv(socketDescriptor, wUsername, len + 1, 0);
        if (nb < 0) {
            perror("[server]Eroare send()3\n");
            close(socketDescriptor);
            exit(1);
        }

        std::cout << wUsername << "\n";
    }
    std::cout << "Max Score = " << maxScore << "\n";

    int myScore;
    nb = recv(socketDescriptor, &myScore, sizeof(int), 0);
    if (nb < 0) {
        perror("[server]Eroare send()3\n");
        close(socketDescriptor);
        exit(1);
    }
    std::cout << "You had " << myScore << " points.\n";
    close(socketDescriptor);
    while(1);
    return 0;
    
}