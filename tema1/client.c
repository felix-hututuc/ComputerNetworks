#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

int main()
{
    int fifoCmd, fifoAns;
    if((fifoCmd = open("./commandFIFO", O_WRONLY)) < 0) {
        perror("Eroare open fifo command");
        exit(-2);
    }
    if((fifoAns = open("./answerFIFO", O_RDONLY)) < 0) {
        perror("Eroare open fifo answer");
        exit(-2);
    }
    
    printf("Welcome! Please log in to the server.\n");
    char command[50];
    while(1) {
        strcpy(command, "");
        int r = read(0, command, 50);
        command[r] = 0;
        size_t l = strlen(command);
        write(fifoCmd, &l, sizeof(long));
        int nbr = write(fifoCmd, command, strlen(command) + 1);
        if(nbr < 0) {
            perror("Eroare write fifo");
            exit(-5);
        }
        sleep(2);
        int ansL;
        nbr = read(fifoAns, &ansL, sizeof(int));
        if(nbr < 0) {
            perror("[1]Eroare read fifo");
            exit(-4);
        }
        char answer[1000];
        nbr = read(fifoAns, answer, ansL);
        if(nbr < 0) {
            perror("[2]Eroare read fifo");
            exit(-4);
        }
        printf("%s", answer);
        if(strncmp(command, "quit", 4) == 0){
            printf("Goodbye!\n");
            break;
        }
    }
    //printf("TEST\n");
    close(fifoAns);
    close(fifoCmd);

    return 0;
}