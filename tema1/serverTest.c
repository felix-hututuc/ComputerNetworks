#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <utmp.h>
#include <time.h>

int logIn(char username[20])
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
    if(mkfifo("./commandFIFO", 0666) < 0) {
        perror("Eroare mkfifo command");
        exit(-1);
    }

    if(mkfifo("./answerFIFO", 0666) < 0) {
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

    int pipefdLogin1[2], pipefdLogin2[2], sockpGLU[2], sockpGPI[2];

    if(pipe(pipefdLogin1) < 0) {
        perror("Eroare deschidere pipe");
        exit(-3);
    }

    if(pipe(pipefdLogin2) < 0) {
        perror("Eroare deschidere pipe");
        exit(-3);
    }

    if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockpGLU) < 0) {
        perror("Eroare deschidere pipe");
        exit(-4);
    }

    if(socketpair(AF_UNIX, SOCK_STREAM, 0, sockpGPI) < 0) {
        perror("Eroare deschidere socketpair");
        exit(-4);
    }

    pid_t pid1;
    if((pid1 = fork()) < 0) {
        perror("Eroare fork1");
        exit(-5);
    }

    if(pid1) { 
        pid_t pid2;
        if((pid2 = fork()) < 0) {
            perror("Eroare fork2");
            exit(-5);
        }
        if(pid2) {
            pid_t pid3;
            if((pid3 = fork()) < 0) {
                perror("Eroare fork3");
                exit(-5);
            }
            if(pid3) {      //parent
                close(pipefdLogin1[0]);
                close(pipefdLogin2[1]); 
                close(sockpGLU[1]);
                close(sockpGPI[1]);

                int logged = 0;
                char command[50];

                while(1) {
                    strcpy(command, "");
                    size_t l;
                    read(fifoCmd, &l, sizeof(long));
                    int nb = read(fifoCmd, command, l + 1);
                    //command[l] = 0;
                    if(nb < 0) {
                        perror("Eroare read comanda");
                        exit(-3);
                    }
                    if(strncmp(command, "login : ", 8) == 0) {
                        if(logged == 0) {
                            nb = write(pipefdLogin1[1], command + 8, l - 8);
                            if(nb < 0) {
                                perror("Eroare write pipe");
                                exit(-7);
                            }
                            sleep(1);
                            int rv;
                            nb = read(pipefdLogin2[0], &rv, sizeof(int));
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
                                int l = 31;
                                write(fifoAns, &l, sizeof(int));
                                write(fifoAns, "27Login failed : bad username\n", 31);
                            }
                        }
                        else {
                            int l = 21;
                            write(fifoAns, &l, sizeof(int));
                            write(fifoAns, "17Already logged in\n", 21);
                        }
                    }
                    else if(strncmp(command, "get-logged-users", 16) == 0) {
                        if(logged == 1) {
                            nb = write(sockpGLU[0], "1", 1);
                            if(nb < 0) {
                                perror("[parent]Eroare write socket");
                                exit(-7);
                            }
                            sleep(1);
                            char answer[1000];
                            nb = read(sockpGLU[0], answer, 1000);
                            if(nb < 0) {
                                perror("[parent]Eroare read socket");
                                exit(-8);
                            }
                            //answer[strlen(answer)] = '\n';
                            answer[strlen(answer)] = '\0';
                            char fullanswer[1100];
                            sprintf(fullanswer, "%ld%s", strlen(answer), answer);
                            int l = strlen(fullanswer) + 1;
                            write(fifoAns, &l, sizeof(int));
                            write(fifoAns, fullanswer, l);
                        }
                        else {
                            int l = 26;
                            write(fifoAns, &l, sizeof(int));
                            write(fifoAns, "22Error : Not logged in.\n", 26);
                        }
                    }
                    else if(strncmp(command, "get-proc-info : ", 16) == 0) {
                        if(logged == 1) {
                            command[l - 1] = 'x';
                            command[l] = '\0';
                            nb = write(sockpGPI[0], command + 16, l - 16);
                            if(nb < 0) {
                                perror("[parent]Eroare write socket");
                                exit(-7);
                            }
                            sleep(1);
                            char answer[1000];
                            strcpy(answer, "");
                            nb = read(sockpGPI[0], answer, 1000);
                            if(nb < 0) {
                                perror("[parent]Eroare read socket");
                                exit(-8);
                            }
                            if(strncmp(answer, "9Wrong pid\n", 11) == 0)
                            {
                                int l = 12;
                                write(fifoAns, &l, sizeof(int));
                                write(fifoAns, answer, l);
                            }
                            else{
                                printf("%s\n", answer);
                                strcat(answer, "\n");
                                printf("%s\n", answer);
                                char fullanswer[1100];
                                sprintf(fullanswer, "%ld%s", strlen(answer), answer);
                                int l = strlen(fullanswer) + 2;
                                write(fifoAns, &l, sizeof(int));
                                write(fifoAns, fullanswer, l);
                            }
                        } 
                        else {
                            int l = 26;
                            write(fifoAns, &l, sizeof(int));
                            write(fifoAns, "22Error : Not logged in.\n", 26);
                        }
                    }
                    else if(strncmp(command, "logout", 6) == 0) {
                        logged = 0;
                        int l = 14;
                        write(fifoAns, &l, sizeof(int));
                        write(fifoAns, "10Logged out\n", 14);
                    }
                    else if(strncmp(command, "quit", 4) == 0) {
                        write(pipefdLogin1[1], "exit", 4);
                        write(sockpGPI[0], "exit", 4);
                        write(sockpGLU[0], "exit", 4);
                        logged = 0;
                        int l = 28;
                        write(fifoAns, &l, sizeof(int));
                        write(fifoAns, "24Server exited succesfuly\n", 28);
                        break;
                    }
                    else {
                        int l = 18;
                        write(fifoAns, &l, sizeof(int));
                        write(fifoAns, "14Unkown command\n", 18);
                    }
                }
                wait(NULL); wait(NULL); wait(NULL);
                
                //printf("Closing parent\n");
                close(pipefdLogin1[1]);
                close(pipefdLogin2[0]); 
                close(sockpGLU[0]);
                close(sockpGPI[0]);
                close(fifoAns);
                close(fifoCmd);

                unlink("./commandFIFO");
                unlink("./answerFIFO");

                exit(0);
            }
            else {          //child3 -- GPI
                close(pipefdLogin1[0]);
                close(pipefdLogin1[1]);
                close(pipefdLogin2[0]);
                close(pipefdLogin2[1]);
                close(sockpGLU[0]);
                close(sockpGLU[1]);
                close(sockpGPI[0]);

                while(1) {
                    char pid[5];
                    int nb = read(sockpGPI[1], pid, 5);
                    if(nb < 0) {
                        perror("[child]Eroare read socket");
                        exit(-1);
                    }
                    if(strncmp(pid, "exit", 4) == 0) break;
                    //pid[strlen(pid) - 2] = 0;
                    int i = 0;
                    while (pid[i] != 'x')
                    {
                        i++;
                    }
                    pid[i] = '\0';
                    char procPath[50];
                    sprintf(procPath, "/proc/%s/status", pid);
                    FILE *fp = fopen (procPath, "r" );
                    rewind(fp);
                    if(fp == NULL) {
                        //perror("error file");
                        nb = write(sockpGPI[1], "9Wrong pid\n", 12);
                        if(nb < 0)
                            perror("[child]Eroare write socket");
                    }
                    else {
                        char answer[1000], line[1000];
                        strcpy(answer, "");
                        while(fgets(line, sizeof(line), fp) != NULL) {
                            if(strncmp(line, "Name:", 5) == 0  ||
                            strncmp(line, "State:", 6) == 0 ||
                            strncmp(line, "PPid:", 5) == 0  ||
                            strncmp(line, "Uid:", 4) == 0   ||
                            strncmp(line, "VmSize:", 5) == 0) {
                                strcat(answer, line);
                            }
                        }
                        answer[strlen(answer) - 1] = '\0';
                        nb = write(sockpGPI[1], answer, strlen(answer) + 1);
                        if(nb < 0)
                            perror("[child]Eroare write socket");
                        fclose(fp);
                    }
                }
                //printf("Closing GPI\n");
                close(sockpGPI[1]);
            }
        }
        else {      //child2 -- GLU
            close(pipefdLogin1[0]);
            close(pipefdLogin1[1]);
            close(pipefdLogin2[0]);
            close(pipefdLogin2[1]);
            close(sockpGLU[0]);
            close(sockpGPI[0]);
            close(sockpGPI[1]);
            
            while(1) {
                char command[10];
                int nb = read(sockpGLU[1], command, 10);
                if(nb < 0) {
                    perror("[child]Eroare read socket");
                    exit(-1);
                }
                if(strncmp(command, "exit", 4) == 0) break;
                char answer[1000];
                strcpy(answer, "");

                setutent();
                struct utmp* utmpStruct;
                while((utmpStruct = getutent()) != NULL) {
                    char line[1000];
                    time_t utmpTime = utmpStruct->ut_time;
                    struct tm *realTime = localtime(&utmpTime);
                    char time[100];
                    sprintf(time, "%d-%d-%d %d:%d\n", realTime->tm_year+1900, realTime->tm_mon+1, realTime->tm_mday, realTime->tm_hour, realTime->tm_min); //login time
                    sprintf(line, "%8s\t%12s\t%s", utmpStruct->ut_user, utmpStruct->ut_host, time);
                    strcat(answer, line);
                }
                nb = write(sockpGLU[1], answer, strlen(answer) + 1);
                if(nb < 0)
                    perror("[child]Eroare write socket");
            }
            
            close(sockpGLU[1]);
            exit(0);
        }
    }
    else {      //child1 -- login
        close(pipefdLogin1[1]);
        close(pipefdLogin2[0]);
        close(sockpGLU[1]);
        close(sockpGLU[0]);
        close(sockpGPI[1]);
        close(sockpGPI[0]);

        while(1) { 
            char username[50];
            int nb;
            nb = read(pipefdLogin1[0], username, 50);
            if(nb < 0) {
                perror("Eroare read pipe");
                exit(-1);
            }
            if(strncmp(username, "exit", 4) == 0) break;
            int x = logIn(username);
            nb = write(pipefdLogin2[1], &x, sizeof(int));
            if(nb < 0)
                perror("Eroare write pipe");
        }

        //printf("Closing login\n");
        close(pipefdLogin1[0]);
        close(pipefdLogin2[1]);
        exit(0);
    }


    return 0;
}