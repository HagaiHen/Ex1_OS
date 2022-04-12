//client function from here: https://aticleworld.com/socket-programming-in-c-using-tcpip/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <dirent.h>

#define SERVER_PORT 5060
#define SERVER_IP_ADDRESS "127.0.0.1"
#define MAX_WORDS_IN_COMMAND 100
#define MAX_CHARS_IN_COMMAND 1000


int client();

void splitCommand(char *str, char** splittedWord) {
    for (int i=0; i<MAX_WORDS_IN_COMMAND; i++) {
        splittedWord[i] = strsep(&str, " ");

        if (splittedWord[i] == NULL) {
            break;
        }
    }
}

int main() {
    char str[MAX_CHARS_IN_COMMAND];
    char cwd[100];
    char* splittedWord[MAX_WORDS_IN_COMMAND];
    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s", cwd);
        }
        printf("/ yes master?\n");
        scanf("%s" , str);
        splitCommand(str,splittedWord);
        if(strcmp(splittedWord[0], "ECHO") == 0){
            printf("%s \n" , splittedWord[1]);
            continue;
        }

        else if(strcmp(splittedWord[0], "DELETE") == 0){    // a system call
            unlink(splittedWord[1]);
            continue;
        }

        else if(strcmp(splittedWord[0], "CD") == 0){
            chdir(splittedWord[1]);
            continue;
        }
        
        else if(strcmp(splittedWord[0], "TCP") == 0){
                client();
                continue;
        }
        else if (!strcmp(str, "EXIT")) {
            exit(0);
        }
        else if (!strcmp(str, "DIR")) {
            DIR *d;
            struct dirent *dir;
            d = opendir(".");
            if (d) {
                while ((dir = readdir(d)) != NULL) {
                    printf("%s\n", dir->d_name);
                }
                closedir(d);continue;
            }
        }
        
        else if(strcmp(splittedWord[0], "COPY") == 0){
            char ch;
            FILE *src, *dst;
            src = fopen(splittedWord[1], "r");
            if(src == NULL){
                perror("Could not open the file");
                return -1;
            }

            dst = fopen(splittedWord[1], "w+");
            if(dst == NULL){
                fclose(src);
                perror("Could not open the file");
                return -1;
            }
            
            while((ch = fgetc(src)) != EOF )
                fputc(ch, dst);
    
            printf("File copied successfully.\n");
            
            fclose(src);
            fclose(dst);
            continue;
        }
        // else{
        //     system(str);    // is a library call from <stdlib. h>
        // }
        else{
            pid_t pid = fork(); 
            if (pid == -1) {   //COULDN'T CREAT CHILD
                printf("\nFailed forking child..");
                continue;
            } 
            else if (pid == 0) {   // CHILD EXECUTE
                if (execvp(splittedWord[0], splittedWord) < 0) {
                    printf("\nCould not execute command..");
                }
                exit(0);
            } else {          // FATHER EXECUTE
                // waiting for child to terminate
                wait(NULL); 
            }
        }
    }return 0;
}

//Create a Socket for server communication
short SocketCreate(void) {
    short hSocket;
    printf("Create the socket\n");
    hSocket = socket(AF_INET, SOCK_STREAM, 0);
    return hSocket;
}

//try to connect with server
int SocketConnect(int hSocket) {
    int iRetval = -1;
    int ServerPort = 90190;
    struct sockaddr_in remote = {0};
    remote.sin_addr.s_addr = inet_addr("127.0.0.1"); //Local Host
    remote.sin_family = AF_INET;
    remote.sin_port = htons(ServerPort);
    iRetval = connect(hSocket, (struct sockaddr *) &remote, sizeof(struct sockaddr_in));
    return iRetval;
}

// Send the data to the server and set the timeout of 20 seconds
int SocketSend(int hSocket, char *Rqst, short lenRqst) {
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 20;  /* 20 Secs Timeout */
    tv.tv_usec = 0;
    if (setsockopt(hSocket, SOL_SOCKET, SO_SNDTIMEO, (char *) &tv, sizeof(tv)) < 0) {
        printf("Time Out\n");
        return -1;
    }
    shortRetval = send(hSocket, Rqst, lenRqst, 0);
    return shortRetval;
}

//receive the data from the server
int SocketReceive(int hSocket, char *Rsp, short RvcSize) {
    int shortRetval = -1;
    struct timeval tv;
    tv.tv_sec = 20;  /* 20 Secs Timeout */
    tv.tv_usec = 0;
    if (setsockopt(hSocket, SOL_SOCKET, SO_RCVTIMEO, (char *) &tv, sizeof(tv)) < 0) {
        printf("Time Out\n");
        return -1;
    }
    shortRetval = recv(hSocket, Rsp, RvcSize, 0);
    printf("Response %s\n", Rsp);
    return shortRetval;
}

int client() {

    int hSocket, read_size;
    struct sockaddr_in server;
    char SendToServer[100] = {0};
    char server_reply[200] = {0};
    char cwd[100];
    //Create socket
    hSocket = SocketCreate();
    if (hSocket == -1) {
        printf("Could not create socket\n");
        return 1;
    }
    printf("Socket is created\n");
    //Connect to remote server
    if (SocketConnect(hSocket) < 0) {
        perror("connect failed.\n");
        return 1;
    }
    printf("Sucessfully conected with server\n");
    //printf("Enter the Message: ");
    while (1) {
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s", cwd);
        }
        printf("\\ yes master?\n");
        gets(SendToServer);
        //Send data to the server
        SocketSend(hSocket, SendToServer, strlen(SendToServer));
        //Received the data from the server
        //read_size = SocketReceive(hSocket, server_reply, 200);
        if (!strcmp(SendToServer, "LOCAL")) {
            break;
        }
    }
    printf("colse");
    close(hSocket);
    shutdown(hSocket, 0);
    shutdown(hSocket, 1);
    shutdown(hSocket, 2);
    return 0;
}