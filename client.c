#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>
#include <time.h>


int main(int argc, char* argv[])
{

    struct sockaddr_in cli_addr , serv_addr;
    int sockfd;
    // Create the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //configure setting of the server address
    memset(&cli_addr, '0', sizeof(cli_addr));
    //address family is the internet
    cli_addr.sin_family = AF_INET;
    // Set port with htons
    cli_addr.sin_port = htons(80);
    // Set IP address to localhost
    //cli_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    cli_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (sockfd == -1){
        perror("socket");
    }
    int len = sizeof(cli_addr);
    if (connect(sockfd, (const struct sockaddr*) &cli_addr, len) == -1){
        perror("connect");
    }
    fprintf(stdout, "Client Online....\n");
    char s[10000];


    
    char quitCmd[] = "quit";
    char getCmd[]  = "get";
    char putCmd[]  = "put";
    char runCmd[]  = "run";
    char listCmd[] = "list";
    char sysCmd[]  = "sys";
    char ack[] = "y"; 
    char nak[] = "n";
    clock_t t;
    time_t start, end, total;
    start = NULL;
    while(1){
        
        char sendBuff[256];
        char recvBuff[256];
        char fileBuff[256];
        
        
        if (start != NULL){
            end = time(0);
            total = end - start;
            printf("ping: %d \n", total);
        }
        // Get initial input
        gets(sendBuff);
        start = time (0);
        
    
        //define request
        char req[strlen(sendBuff)];
        strcpy(req, sendBuff);

        char *token;
        char *cmd;
        // ToDo make arguements dynamic
        char *arguments[5];
        // First token will be the command
        token = strtok(req, " ");
        cmd = token;

        int ctr = 0;
        while (token != NULL){
            token = strtok(NULL, " ");
            arguments[ctr] = token;
            ctr++;     
        }
        ctr--;


    // case statement for any type input copmmand
        if (strcmp(cmd, quitCmd)==0){
            send(sockfd,"quit",256,0);
            close(sockfd);
            break;
        } else if (strcmp(cmd, getCmd)==0){
            if (ctr != 2){
                printf("Invalid arguments: get 'progname' 'sourcefile'");
                break;
            }
            // Send command to the server
            send(sockfd,sendBuff,256,0);

            while (1){
                // Get line of progname
                recv(sockfd, &recvBuff, sizeof(recvBuff), 0);
                //send to acknowledge
                send(sockfd, ack, 256,0);
                // If the server reaches eof break
                if (strcmp(recvBuff, "eof") == 0){
                    printf("eof\n");
                    break;
                // If the server hits 40 lines read in
                } else if (strcmp(recvBuff, "pause") == 0){
                    printf("press any key to continue:");
                    char c;
                    scanf(" %c", &c);
                    printf("\n");
                    send(sockfd, "ys", 256,0);
                // Else print the incoming line
                } else {
                    printf("-> %s\n", recvBuff);
                }            
            }
            break;


        } else if (strcmp(cmd, putCmd)==0){
            // If there are too little commands break
            if (ctr < 2){
                printf("Invalid arguments: put 'progname' 'sourcefile'[s] [-f]");
                break;
            }
            
            int numfiles = ctr - 1;
            // If the force flag has been sent reduce a number from how many source files have been passed
            if (strcmp(arguments[ctr - 1], "-f") == 0){
                numfiles--;
            }
        
            // send command and arguments
            send(sockfd,sendBuff,256,0);
            // Recieve error or go ahead to send files
            recv(sockfd, &recvBuff, 256, 0);
            if (strcmp(recvBuff, "y") != 0){
                puts(recvBuff);
                break;
            }
        
            // For number of files send to server
            // TODO turn into while loop
            for (int i = 1; i < numfiles + 1; i++){
                char filedir [50];
                strcpy(filedir, arguments[i]);

                FILE *fp = fopen(filedir, "rb");
                if (fp == NULL){
                    perror("File not found");
                    break;
                }
                int byteCount;
                byteCount = fread(fileBuff, 1, sizeof(fileBuff), fp);
                send(sockfd, fileBuff, byteCount, 0);
                fclose(fp);
                // ackowledge upload
                recv(sockfd, &recvBuff, 256, 0);
                puts(recvBuff);
            }

        } else if (strcmp(cmd, runCmd)==0){
            //not even sure
            send(sockfd,"run progname [args] [-f localfile]",256,0);
        } else if (strcmp(cmd, listCmd)==0){
            // possibly just check if 1 or 2 args then send full request
            if (ctr > 2){
                printf("Invalid arguments: list [-l] [progname]\n");
                break;
            } 
            send(sockfd, sendBuff, 256, 0);
            
            while(1){
            recv(sockfd, &recvBuff, 256, 0);
            if (strcmp(recvBuff, "q")==0){
                    break;
                }
            printf("-> %s\n", recvBuff);
            send(sockfd, "g", 256, 0);
                
                
            }
        } else if (strcmp(cmd, sysCmd)==0){
            // Send command straight to server
            // Maybe verify there are no arguements
            send(sockfd,sendBuff,256,0);
            recv(sockfd, &recvBuff, 256, 0);
            send(sockfd, "ack", 256, 0);
            printf("System: %s\n", recvBuff);
            recv(sockfd, &recvBuff, 256, 0);
            send(sockfd, "ack", 256, 0);
            printf("Version: %s\n", recvBuff);
            recv(sockfd, &recvBuff, 256, 0);
            send(sockfd, "ack", 256, 0);
            printf("CPU type: %s\n", recvBuff);
            recv(sockfd, &recvBuff, 256, 0);
        }  else {
            printf("Command not found\n");
    }
    }

    //gather all inputs as needed


    close(sockfd);
    return 0;

}