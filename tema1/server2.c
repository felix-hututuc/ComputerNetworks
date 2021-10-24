#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>

int login(char username[20])
{
    FILE *fp = fopen ( "./users.txt", "r" );

    if (fp != NULL) {
        char user[1000];
        while(fgets(user,sizeof(user),fp) != NULL) {
            if(strcmp(username, user) == 0) 
                return 1;
        }

        fclose(fp);
    }
    else {
        perror("Eroare fisier usernames");
    }

    return 0;
}

int main()
{
    if(mkfifo("./commandFIFO", 0666) == -1) {
        perror("Eroare mkfifo command");
        exit(-1);
    }

    if(mkfifo("./answerFIFO", 0666) == -1) {
        perror("Eroare mkfifo answer");
        exit(-1);
    }

    int fifoCmd, fifoAns;
    if((fifoCmd = open("./commandFIFO", O_RDONLY)) < 0) {
        perror("Eroare open fifo command");
        exit(-1);
    }

    if((fifoAns = open("./answerFIFO", O_WRONLY)) < 0) {
        perror("Eroare open fifo answer");
        exit(-2);
    }

    int logged = 0;
    char command[50];
    while(1) {
        strcpy(command, "");
        size_t l;
        read(fifoCmd, &l, sizeof(long));
        //printf("%ld", l);
        int nb = read(fifoCmd, command, l);
        command[l] = 0;
        //if(nb != l) perror("problema");
        if(nb < 0) {
            perror("Eroare read comanda");
            exit(-3);
        }
        //printf("%s\n", command);
        if(strncmp(command, "login : ", 8) == 0) {
            if(logged == 0) {
                int pipefdLogin[2];
                if(pipe(pipefdLogin) < 0) {
                    perror("Eroare pipe login");
                    exit(-4);
                }
                pid_t pid = fork();
                if(pid < 0) {
                    perror("Eroare fork");
                    exit(-5);
                }
                if(pid != 0) {
                    close(pipefdLogin[1]);
                    int rv;
                    wait(NULL);
                    nb = read(pipefdLogin[0], &rv, sizeof(int));
                    if(nb < 0) {
                        perror("Eroare read pipe");
                        exit(-6);
                    }
                    if(rv == 1) {
                        int l = 20;
                        write(fifoAns, &l, sizeof(int));
                        write(fifoAns, "16Login succesful!\n", 20);
                        logged = 1;
                    }
                    else if(rv == 0){
                        int l = 16;
                        write(fifoAns, &l, sizeof(int));
                        write(fifoAns, "13Login failed\n", 16);
                    }
                    close(pipefdLogin[0]);
                }
                else {
                    close(fifoAns);
                    close(fifoCmd);
                    close(pipefdLogin[0]);
                    int x = login(command + 8);
                    nb = write(pipefdLogin[1], &x, sizeof(x));
                    if(nb < 0) {
                        perror("Eroare write pipe");
                    }
                    close(pipefdLogin[1]);
                    exit(0);
                }
            }
            else {
                int l = 21;
                write(fifoAns, &l, sizeof(int));
                write(fifoAns, "17Already logged in\n", 21);
            }
        }
        if(strncmp(command, "get-logged-users", 16) == 0) {
            if(logged == 1) {
                int socketfd[2];
                if(socketpair(AF_UNIX, SOCK_STREAM, 0, socketfd) < 0) {
                    perror("Eroare deschidere socketpair");
                    exit(-4);
                }
            }
            else {
                int l = 26;
                write(fifoAns, &l, sizeof(int));
                write(fifoAns, "22Error : Not logged in.\n", 26);
            }
        }
        if(strncmp(command, "get-proc-info : ", 16) == 0) {
            if(logged == 1) {
                int socketfd[2];
                if(socketpair(AF_UNIX, SOCK_STREAM, 0, socketfd) < 0) {
                    perror("Eroare deschidere socketpair");
                    exit(-4);
                }
                pid_t pid = fork();
                if(pid < 0) {
                    perror("Eroare fork");
                    exit(-5);
                }
                if(pid) {
                    close(socketfd[1]);
                }
                else {
                    close(socketfd[0]);
                }
            }
            else {
                int l = 26;
                write(fifoAns, &l, sizeof(int));
                write(fifoAns, "22Error : Not logged in.\n", 26);
            }
        }
        if(strncmp(command, "logout", 6) == 0) {
            logged = 0;
            int l = 14;
            write(fifoAns, &l, sizeof(int));
            write(fifoAns, "10Logged out\n", 14);
        }
        if(strncmp(command, "quit", 4) == 0) {
            //printf("TEST\n");
            logged = 0;
            int l = 28;
            write(fifoAns, &l, sizeof(int));
            write(fifoAns, "24Server exited succesfuly\n", 28);
            close(fifoCmd);
            close(fifoAns);
            break;
        }
        else {
            int l = 26;
            write(fifoAns, &l, sizeof(int));
            write(fifoAns, "22Error: Unknown command\n", 26);
        }
    }




    unlink("./commandFIFO");
    unlink("./answerFIFO");

    return 0;
}