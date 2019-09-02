#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/utsname.h>

#define MAXLEN 50

void error(char *msg){
    perror(msg);
    exit(1);
}

void removeDir(char progdir[] ){
    DIR *dir_ptr;
    struct dirent *direntp;
    if ((dir_ptr = opendir(progdir)) == NULL){
        fprintf(stderr, "ls: can't open %s\n", progdir);
        } else {
            while ((direntp = readdir(dir_ptr)) != NULL){
                char filename[50];
                strcpy(filename, direntp->d_name);
                if (strcmp(filename, ".")!=0 && strcmp(filename, "..")!=0){
                    printf("filename %s\n", filename);
                    char todel[100];
                    strcpy(todel,progdir );
                    
                    strcat(todel, "/");
                    strcat(todel, filename);
                    printf("%s \n", todel);
                    int ret = remove(todel);
                    if(ret == 0) {
                        printf("File deleted successfully");
                    } else {
                        printf("Error: unable to delete the file\n");
                    }
                }
            }
            closedir(dir_ptr);
        }
}


int main(){

    struct sockaddr_in serv_addr ,cli_addr;
    int sockfd, newsockfd;

    // create the socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // replace bzero since its depreciated
    // 
    memset(&serv_addr, '0', sizeof(serv_addr));

    /* Configure settings of the server address struct */
    // Address Family = Internet
    serv_addr.sin_family = AF_INET;
    // set port number using htons  // dynamic portno look at atoi(arg)
    serv_addr.sin_port = htons(80);
    // Set IP address to localhost
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (sockfd==-1){
        error("ERROR opening socket");
    }
    int len=sizeof(serv_addr);
    // Bind adderss struct to the socket
    if (bind(sockfd, ( struct sockaddr*)&serv_addr, len) == -1){
        error("ERROR on binding");
    }
    // Listen for connections
    if (listen(sockfd, 10) == -1){
        error("ERROR on listening");
    }
    int pid; //,newsockfd;
    static int counter = 0;
    char quitCmd[] = "quit";
    char getCmd[]  = "get";
    char putCmd[]  = "put";
    char runCmd[]  = "run";
    char listCmd[] = "list";
    char sysCmd[]  = "sys";
    char ack[] = "y";
    char nak[] = "n";

    //wait for clients
    while(1){
        //accept connections
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &len);
        pid = fork();
        //printf("pid = %i\n", pid);
        // if error
        if (pid == -1){
            close(newsockfd);
            continue;
        }
        //if parent process
        else if(pid > 0){
            close(newsockfd);
            counter++;
            continue;
        }
        // if child process
        else if(pid == 0){
            int byteCount;
            // wait for commands
            while(1){
                char sendBuff[256];
                char recvBuff[256];
                
                    recv(newsockfd, &recvBuff, 256, 0);
                    puts(recvBuff);
                    // Check for command
                    char command[strlen(recvBuff)];
                    strcpy(command, recvBuff);
                    //empty buffer
                    memset(&recvBuff, '\0', sizeof(recvBuff));
                    
                    //strtok ' '
                    // dynamic 2D array 
                    char *token;
                    char *cmd;
                    char *arguments[5];
                    //get first token
                    token = strtok(command, " ");
                    printf("cmd %s\n", token);
                    cmd = token;
                    //args = strtok(command, " ");
                    // while there are more tokens
                    int ctr = 0;
                    while (token != NULL){
                        token = strtok(NULL, " ");
                        printf("token %s\n", token);
                        arguments[ctr] = token; 
                        ctr++;
                    }
                    ctr--;
                    printf("cmd %i\n", strcmp(cmd, listCmd));
                    // case statement for any type of string compare
                    if (strcmp(cmd, quitCmd)==0){
                        close(newsockfd);
                        break;
                    } else if (strcmp(cmd, getCmd)==0){
                        // turn arguments in file directory
                        //TODO move into function
                        char filedir[] = "./prog/";
                        char progname [sizeof(arguments[0])];
                        char currentfile [sizeof(arguments[1])];
                        printf("aeg: %s %i\n",arguments[1], strlen(arguments[1]));
                        strcpy(progname, arguments[0]);
                        printf("progname: %s\n",progname);
                        strcpy(currentfile, arguments[1]);
                        printf("current: %s\n",currentfile);
                        

                        strcat(filedir, arguments[0]);
                        printf("filedir: %s\n",filedir);
                        strcat(filedir,"/");
                        printf("filedir: %s\n",filedir);
                        
                        strcat(filedir, currentfile);
                        printf("filedir: %s\n",filedir);

                        
                        FILE *fp;
                        char *line = NULL;
                        size_t len = 0;
                        ssize_t read;

                        // Open file
                        fp = fopen(filedir, "r");
                        if (fp == NULL) {
                            send(newsockfd,"Unable to open file",256,0);
                            break;
                        }
                        
                        int count=1;
                        char ackBuf[265];
                        //read file line by line
                        while ((read = getline(&line, &len, fp)) != -1) {
                            //if counter % 4 == 0 wait for keypress
                            printf("%s", line);
                            // Send a single line of the file to client
                            send(newsockfd, line, 256,0);
                            // Acknowledge the file has been recieved
                            recv(newsockfd, &recvBuff, sizeof(recvBuff), 0);
                            // if reading hits max number of lines
                            if (count % 40 == 0 && count != 0){
                                // send pause command to client
                                send(newsockfd, "pause", 256, 0);
                                // reset buffer
                                memset(&ackBuf, '\0', sizeof(ackBuf));
                                // Wait for a response to continue
                                while (1){
                                    printf("%s ", ackBuf);
                                    if (strcmp(ackBuf, "ys")==0){
                                        break;
                                    }
                                    recv(newsockfd, &ackBuf, sizeof(ackBuf), 0);
                                }
                                count = 0;
                            }
                            count++;
                        }
                        send(newsockfd, "eof", 256, 0);
                        fclose(fp);
                        
                    } else if (strcmp(cmd, putCmd)==0){

                        char f = 0;
                        struct stat st = {0};
                        int numfiles = ctr - 1;
                        // create dir based on progname
                        char progdir[] = "./prog/";
                        char progname[50];
                        strcpy(progname, arguments[0]);
                        strcat(progdir, progname);
                        
                        //print test all variables
                        printf("Force: %i, progname: %s, prog dir: %s, number of sourcefiles: %i\n", f,progname, progdir, numfiles);
                        printf("%s should be -f", arguments[ctr - 1]);
                        //if dir doesn't exist
                        if (stat(progdir, &st) == -1) {
                            // make directory
                            mkdir(progdir, 0700);
                        } else {
                            //if the force argument was passed through
                            if ( strcmp(arguments[ctr - 1], "-f") == 0){
                                f = 1;
                                numfiles--;
                                //remove files in dir
                                removeDir(progdir);
                            } else {
                                //send error and break
                                send(newsockfd,"error directory already exists", 256, 0);
                                break;
                            }
                        }
                        // Send confirmation to send files
                        send(newsockfd, ack, sizeof(ack), 0);
                        //recieve file
                        printf("%i", numfiles);
                        for (int i = 1; i < numfiles + 1; i++){
                            char currentfile [50];
                            char filedir[50];
                            strcpy(currentfile, arguments[i]);
                            strcpy(filedir, progdir);
                            strcat(filedir,"/");
                            strcat(filedir, currentfile);


                            FILE* fp = fopen( filedir, "wb");
                    
                            if (fp != NULL){
                                byteCount = recv(newsockfd, &recvBuff, 256,0);
                                fwrite(recvBuff, 1, byteCount, fp);
                                send(newsockfd,"file write complete",256,0);
                                if (byteCount<0){
                                    perror("Receiving");
                                }
                                fclose(fp);
                            } else {
                                perror("File");
                            }
                        }

                    } else if (strcmp(cmd, runCmd)==0){
                        system("gcc -o test test.c");
                        char ps[100];
                        char p;
                        int count=0;
                        FILE* proc = popen("./test.exe", "r");
                        while((p=fgetc(proc))!=EOF){
                            ps[count] = p;
                            count++;
                        }
                        printf("final output: %s ",ps);
                        send(newsockfd, ps, 256, 0);
                        pclose(proc);
                        
                        

                    } else if (strcmp(cmd, listCmd)==0){

                        DIR *dir_ptr;
                        struct dirent *direntp;
                        char dirname[] = "./prog";
                        if (ctr == 2){
                            printf("long lsit in dir %s \n", dirname);
                            strcat(dirname, "/");
                            strcat(dirname, arguments[0]);
                        } else if (strcmp(arguments[ctr - 1], "-l") != 0){
                            strcat(dirname, "/");
                            strcat(dirname, arguments[0]);
                            printf("short lsit prog %s \n", dirname);
                        } 

                        printf("dirname %s",dirname);

                        if ((dir_ptr = opendir(dirname)) == NULL){
                            fprintf(stderr, "ls: can't open %s\n", dirname);
                        } else {
                            while ((direntp = readdir(dir_ptr)) != NULL){
                                char filename[50];
                                char filedir[50];
                                
                                strcpy(filename, direntp->d_name);
                                strcpy(filedir, dirname);
                                strcat(filedir, "/");
                                strcat(filedir, filename);
                                //printf("filedir: %s", filedir);

                                if (strcmp(filename, ".")!=0 && strcmp(filename, "..")!=0){
                                    if (arguments[0] != NULL && strcmp(arguments[ctr - 1], "-l") == 0){
                                        struct stat sfile;
                                        if (stat(filedir, &sfile) == -1){
                                            printf("Error Occurred\n");
                                        }
                                        char longlist[100];
                                        strcpy(longlist, filename);
                                        strcat(longlist, " ");
                                        char created[50];
                                        
                                        int length = snprintf( NULL, 0, "%d", sfile.st_size );
                                        char* str = malloc( length + 1 );
                                        snprintf( str, length + 1, "%i", sfile.st_size );
                                        char fsize [50];
                                        strcpy(fsize, str);
                                        strcat(longlist, fsize);
                                        
                                        strcat(longlist, " "); 
                                        char perms[4];
                                        if (sfile.st_mode && S_IRUSR){
                                            perms[0]='r';
                                        } else {
                                            perms[0]='-';
                                        }
                                        if (sfile.st_mode && S_IWUSR){
                                            perms[1]='w';
                                        } else {
                                            perms[1]='-';
                                        }
                                        if (sfile.st_mode && S_IXUSR){
                                            perms[2]='x';
                                        } else {
                                            perms[2]='-';
                                        }
                                        perms[3] = '\0';
                                        strcat(longlist,perms);
                                        strcat(longlist, " ");
                                        strcpy(created, ctime(&sfile.st_mtime));
                                        strcat(longlist, created);
                                        send(newsockfd, longlist, 256, 0);
                                        recv(newsockfd, &recvBuff, 256, 0);
                                        
                                    }else {
                                        printf("filename: %s\n", filename);
                                        send(newsockfd, filename, 256, 0);
                                        recv(newsockfd, &recvBuff, 256, 0);
                                    } 
                                } 
                                
                            }
                            send(newsockfd, "q", 256,0);
                            closedir(dir_ptr);
                        }
                    } else if (strcmp(cmd, sysCmd)==0){
                        struct utsname unameData;
                        
                        uname(&unameData);
                        printf("System: %s \nVersion: %s\nMachine harware: %s\nRelease: %s\n", unameData.sysname, unameData.version, unameData.machine, unameData.release);
                        send(newsockfd, unameData.sysname, sizeof(unameData.sysname),0);
                        recv(newsockfd, &recvBuff, 256, 0);
                        send(newsockfd, unameData.machine, sizeof(unameData.release),0);
                        recv(newsockfd, &recvBuff, 256, 0);
                        send(newsockfd, unameData.release, sizeof(unameData.machine),0);
                        recv(newsockfd, &recvBuff, 256, 0);
                        send(newsockfd, "ack", 256, 0);
    
    
                        //send(newsockfd,"probably unix",256,0);
                    } else {
                        send(newsockfd,"command not found",256,0);
                    }
                
            }
            close(newsockfd);
            break;
        }
}
    close(sockfd);
    return 0;
}