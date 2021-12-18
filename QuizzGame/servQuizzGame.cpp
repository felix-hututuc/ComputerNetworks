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
#include <algorithm>
#include <random>
#include <time.h>
#include "Player.h"

#define PORT 2022

extern int errno;
const char* IP = "192.168.1.231";
const char* DIR = "./db/QUIZZ.db";
bool acceptPlayers = true, openServer = true;
int numberOfPlayers = 0, numberOfThreads = 0;
int timeToAnswer = 30;
std::vector<Player> players;
std::vector<int> questionsIndex;
pthread_mutex_t mutexPlayerVec;
sqlite3* DB;


struct ThreadArg {
    int sd;
    int threadNo;
};

struct Question {
    std::string question, ans1, ans2, ans3, ans4;
    char corAns;
};

void sigHandler(int sign)
{
    if(sign == SIGINT) {
        openServer = false;
        printf("Exiting server\n");
    }

    if(sign == SIGPIPE) {
        printf("A player left the game.\n");
        numberOfThreads--;
    }
}

static int createDB()
{

    std::string sql = "CREATE TABLE IF NOT EXISTS QUIZZ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "QUESTION VARCHAR2(100) NOT NULL, "
        "ANS1 VARCHAR2(50) NOT NULL, "
        "ANS2 VARCHAR2(50) NOT NULL, "
        "ANS3 VARCHAR2(50) NOT NULL, "
        "ANS4 VARCHAR2(50) NOT NULL, "
        "CORANS CHAR(1) NOT NULL );";

    char* errorMsg;
    int ret = 0;
    ret = sqlite3_exec(DB, sql.c_str(), NULL, 0, &errorMsg);
    if (ret != SQLITE_OK) {
        perror("Eroare creare tabel\n");
        sqlite3_free(errorMsg);
    } else {
        std::cout << "DB and Table created succesfuly\n";
    }

    return 0;
}

static int insertData()
{
    char* errorMsg;

    int ret;

    std::string sql("INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('What protocol is used to find the hardware address of a local device?', 'RARP', 'ARP', 'IP', 'ICMP', 'B');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('Which of the following animals can run the fastest?', 'Cheetah', 'Leopard', 'Tiger', 'Lion', 'A');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('What is the most points that a player can score with a single throw in darts?', '20', '40', '60', '80', 'C');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('In which country is Transylvania?', 'Bulgaria', 'Romania', 'Croatia', 'Serbia', 'B');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('Which of the following protocols uses both TCP and UDP?', 'FTP', 'SMTP', 'Telnet', 'DNS', 'D');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('Which port number is used by HTTP ?', '23', '80', '53', '110', 'B');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('Which characteristic is a part of TCP?', 'Reliable', 'Connectionless', 'No flow control', 'Uses datagrams', 'A');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('Which of the following services is used to translate a web address into an IP address?', 'DNS', 'WINS', 'DHCP', 'Telnet', 'A');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('Which protocol is used by FTP to transfer files over the Internet?', 'SMTP', 'UDP', 'SNMP', 'TCP', 'D');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('Which port number is used by SMTP ?', '20', '23', '25', '143', 'C');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('What is the purpose of TCP/UDP port numbers ?', 'To indicate the beginning of a three-way handshake', 'To reassemble the segments into the correct order', 'To identify the numbers of the data packets that can be sent without acknowledgment', 'To track the different conversations crossing the network at the same time', 'D');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('Which layer 4 protocol is used for a Telnet connection?', 'IP', 'TCP', 'TCP/IP', 'UDP', 'B');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('If you use either Telnet or FTP, which is the highest layer you are using to transmit data?', 'Application', 'Presentation', 'Session', 'Transport', 'A');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('What layer in the TCP/IP stack is equivalent to the Transport layer of the OSI model?', 'Application', 'Host-to-Host', 'Internet', 'Network Access', 'B');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('Which protocol does Ping use?', 'TCP', 'ARP', 'ICMP', 'BootP', 'C');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('Segmentation of a data stream happens at which layer of the OSI model?', 'Physical', 'Data Link', 'Network', 'Transport', 'D');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('The arrangement of the computers in a network is called the', 'NOS', 'Topology', 'Node layout', 'Protocol', 'B');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('Any device that is connected to a network is called a', 'Client', 'Node', 'Server', 'Manager', 'B');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('A small network setup in your home is called a', 'Hub network', 'Center network', 'Station network', 'Local area network', 'D');"
    "INSERT INTO QUIZZ (QUESTION, ANS1, ANS2, ANS3, ANS4, CORANS) VALUES('If the nodes can serve as both servers and clients, the network is said to be', 'Hybrid', 'Terminal', 'Peer-to-peer', 'Hierarchical', 'C');"
    );

    ret = sqlite3_exec(DB, sql.c_str(), NULL, 0, &errorMsg);
    if (ret != SQLITE_OK) {
        perror("Eroare insert in db\n");
        sqlite3_free(errorMsg);
    } else {
        std::cout << "Data inserted succesfuly\n";
    }

    return 0;
}

// static int callback(void* NotUsed, int argc, char** argv, char** azColName)
// {
//     for (int i = 0; i < argc; i++) {
//         std::cout << azColName[i] << ": " << argv[i] << std::endl;
//     }

//     std::cout << std::endl;

//     return 0;
// }

Question* selectQuestion(int index)
{
    int ret;

    std::string strIndex = std::to_string(index);
    std::string sql = "SELECT question, ans1, ans2, ans3, ans4, corans FROM QUIZZ WHERE ID = ";
    sql = sql + strIndex + ";";

    // ret = sqlite3_exec(DB, sql.c_str(), callback, NULL, &errorMsg);

    sqlite3_stmt* sqlStmt;
    ret = sqlite3_prepare_v2(DB, sql.c_str(), sql.size() + 1, &sqlStmt, NULL);
    if (ret != SQLITE_OK) {
        perror("Eroare select db");
        exit(-1);
    }

    ret = sqlite3_step(sqlStmt);
    if (ret == SQLITE_ERROR) {
        perror("Eroare select db");
        exit(-2);
    }

    Question* quiz = (Question*)malloc(sizeof(Question));
    unsigned char column[100];
    //column = sqlite3_column_text(sqlStmt, 0);
    strcpy((char*)column, (char*)sqlite3_column_text(sqlStmt, 0));
    quiz->question = (char*) column;
    strcpy((char*)column, (char*)sqlite3_column_text(sqlStmt, 1));
    quiz->ans1 = (char*) column;
    strcpy((char*)column, (char*)sqlite3_column_text(sqlStmt, 2));
    quiz->ans2 = (char*) column;
    strcpy((char*)column, (char*)sqlite3_column_text(sqlStmt, 3));
    quiz->ans3 = (char*) column;
    strcpy((char*)column, (char*)sqlite3_column_text(sqlStmt, 4));
    quiz->ans4 = (char*) column;
    strcpy((char*)column, (char*)sqlite3_column_text(sqlStmt, 5));
    quiz->corAns = column[0];
    return quiz;
}

void shuffleQuestions()
{
    for (int i = 0; i < 20; i++) {
        questionsIndex.push_back(i + 1);
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(questionsIndex.begin(), questionsIndex.end(), g);
}

bool isPlayer(const std::string& username) 
{
    for (auto p : players) {
        if(p.getUsername() == username)
            return true;
    }
    return false;
}   

int sendQuestion(const int& clientSD, const Question& quiz)
{
    int len, nb;

    // Question
    len = quiz.question.size();
    nb = send(clientSD, &len, sizeof(int), 0);
    if(nb < 0) {
        perror("[server]Eroare send()\n");
        close(clientSD);
        exit(-1);
    }
    nb = send(clientSD, quiz.question.c_str(), len + 1, 0);
    if(nb < 0) {
        perror("[server]Eroare send()\n");
        close(clientSD);
        exit(-1);
    }

    //Answer A
    len = quiz.ans1.size();
    nb = send(clientSD, &len, sizeof(int), 0);
    if(nb < 0) {
        perror("[server]Eroare send()\n");
        close(clientSD);
        exit(-1);
    }
    nb = send(clientSD, quiz.ans1.c_str(), len + 1, 0);
    if(nb < 0) {
        perror("[server]Eroare send()\n");
        close(clientSD);
        exit(-1);
    }

    //Answer B
    len = quiz.ans2.size();
    nb = send(clientSD, &len, sizeof(int), 0);
    if(nb < 0) {
        perror("[server]Eroare send()\n");
        close(clientSD);
        exit(-1);
    }
    nb = send(clientSD, quiz.ans2.c_str(), len + 1, 0);
    if(nb < 0) {
        perror("[server]Eroare send()\n");
        close(clientSD);
        exit(-1);
    }

    //Answer C
    len = quiz.ans3.size();
    nb = send(clientSD, &len, sizeof(int), 0);
    if(nb < 0) {
        perror("[server]Eroare send()\n");
        close(clientSD);
        exit(-1);
    }
    nb = send(clientSD, quiz.ans3.c_str(), len + 1, 0);
    if(nb < 0) {
        perror("[server]Eroare send()\n");
        close(clientSD);
        exit(-1);
    }

    //Answer D
    len = quiz.ans4.size();
    nb = send(clientSD, &len, sizeof(int), 0);
    if(nb < 0) {
        perror("[server]Eroare send()\n");
        close(clientSD);
        exit(-1);
    }
    nb = send(clientSD, quiz.ans4.c_str(), len + 1, 0);
    if(nb < 0) {
        perror("[server]Eroare send()\n");
        close(clientSD);
        exit(-1);
    }
    
    return 0;
}

static void* playerRoutine(void* args) {
    ThreadArg client = *(ThreadArg*) args;
    //int client = *(int*) args;
    int len, nb;
    bool loggedIn = 0;
    std::string strUsername;
    do {
        nb = recv(client.sd, &len, sizeof(int), 0);
        if(nb < 0) {
            perror("[server]Eroare recv()\n");
            close(client.sd);
            exit(1);
        }
        char* username = new char[len + 1];
        nb = recv(client.sd, username, len + 1, 0);
        if(nb < 0) {
            perror("[server]Eroare recv()\n");
            close(client.sd);
            exit(1);
        }
        strUsername = username;
        delete(username);

        pthread_mutex_lock(&mutexPlayerVec);
        //search for username
        if(!isPlayer(strUsername)) {              //username not found
            loggedIn = 1;
        }
        pthread_mutex_unlock(&mutexPlayerVec);
        nb = send(client.sd, &loggedIn, sizeof(bool), 0);
        if(nb < 0) {
            perror("[server]Eroare send()\n");
            close(client.sd);
            exit(1);
        }
    }while(!loggedIn);

    Player player(strUsername);
    int indexInVec;

    pthread_mutex_lock(&mutexPlayerVec);
    indexInVec = players.size();
    players.push_back(player);
    numberOfPlayers++;
    pthread_mutex_unlock(&mutexPlayerVec);

    char receivedChar;
    nb = recv(client.sd, &receivedChar, 1, 0);
    if(nb < 0) {
        perror("[server]Eroare recv() char\n");
        close(client.sd);
        exit(1);
    }

    int index = 0;
    Question* quiz;

    quiz = selectQuestion(questionsIndex[index]);

    sendQuestion(client.sd, *quiz);

    time_t startTime = time(0);
    time_t end, timeTaken;
    int timeLeft = timeToAnswer;
    bool answered = false;
    while (timeLeft > 0 && !answered){
        nb = recv(client.sd, &receivedChar, 1, MSG_DONTWAIT);
        // if (nb < 0) {
        //     perror("[server]Eroare recv() char\n");
        //     close(client.sd);
        //     exit(5);
        // }
        if (nb == 1) {
            answered = true;
            break;
        }
        end = time(0);
        timeTaken = end - startTime;
        timeLeft = timeToAnswer - timeTaken;
    }
    if (!answered) {
        std::cout << "Time expired\n";
        nb = send(client.sd, &loggedIn, sizeof(bool), 0);
        if(nb < 0) {
            perror("[server]Eroare send()\n");
            close(client.sd);
            exit(1);
        }
    } else {
        nb = recv(client.sd, &receivedChar, 1, 0);
        if (nb < 0) {
            perror("[server]Eroare recv() char\n");
            close(client.sd);
            exit(5);
        }
        std::cout << receivedChar - 32 << "\n";
        std::cout << quiz->corAns << "\n";
        if(quiz->corAns == (char)(receivedChar - 32)) {
            players[indexInVec].incScore();
            std::cout << "Correct answer\n";
        } else {
            std::cout << "Wrong answer\n";
        }
    }
    
    close(client.sd);
    free(args);
    free(quiz);
    pthread_exit(NULL);
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
        if (sqlite3_open(DIR, &DB) < 0) {
            perror("Eroare open DB\n");
            return 3;
        }
        createDB();
        // insertData();

        pthread_t* playerThreads;
        pthread_attr_t detachedThreadAttr;
        pthread_attr_init(&detachedThreadAttr);
        pthread_attr_setdetachstate(&detachedThreadAttr, PTHREAD_CREATE_DETACHED);
        pthread_mutex_init(&mutexPlayerVec, NULL);

        // struct sigaction newHandler;
        // struct sigaction oldHandler;
        // memset(&newHandler, 0, sizeof(newHandler));
        // newHandler.sa_handler = sigHandler;
        // sigemptyset(&newHandler.sa_mask);
        // newHandler.sa_flags = 0;
        // if(sigaction(SIGINT, &newHandler, &oldHandler) < 0) {
        //     perror("[server]Eroare signal.\n");
        //     return errno;
        // }

        // if(signal(SIGINT, sigHandler) == SIG_ERR) {
        //     perror("[server]Eroare signal.\n");
        //     return errno;
        // }

        if(signal(SIGPIPE, sigHandler) == SIG_ERR) {
            perror("[server]Eroare signal.\n");
            return errno;
        }

        shuffleQuestions();

        struct sockaddr_in servInfo;
        struct sockaddr_in clInfo;
        int socketDescriptor;

        if((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("[servInfo]Eroare socket().\n");
            return errno;
        }

        int on = 1;
        setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

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
                playerThreads[numberOfThreads] = (pthread_t)malloc(sizeof(pthread_t));
                ThreadArg* args = (ThreadArg*) malloc(sizeof(ThreadArg));
                args->sd = client;
                args->threadNo = numberOfThreads;
                numberOfThreads++;
                if(pthread_create(&playerThreads[numberOfThreads], &detachedThreadAttr, &playerRoutine, args) != 0) {
                    perror("[server]Failed to create thread.\n");
                    return errno;
                }
                continue;
            }
        }

        pthread_attr_destroy(&detachedThreadAttr);
        pthread_mutex_destroy(&mutexPlayerVec);

        exit(0);
    }
}
