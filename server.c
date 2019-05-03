#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

int globalsockfd;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void signalhandler(int signum){
    printf("Caught signal %d\n",signum);
    printf("Closing socket\n");
    close(globalsockfd);
    printf("Killing All Processes:\n");
    kill(0, SIGKILL);
    exit(signum);
    
}



int main(int argc, char *argv[]){
    
   // signal(SIGINT,signalhandler(SIGINT,sockfd));
    int sockfd, newsockfd, portno;
    signal(SIGINT,signalhandler);
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    int status,status2;
    char str[INET_ADDRSTRLEN];
    int grandchildpid,childpid;
    if (argc < 2)
    {
        fprintf(stderr, "No port provided\n");
        exit(1);
    }
            /*Create Socket*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    globalsockfd=sockfd;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
            /* Bind Socket */
    if (bind(sockfd,(struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    while(1){
        if(listen(sockfd, 5)==-1){
            perror("listen");   
        }
        clilen = sizeof(cli_addr);//Creating child process to connect
        printf("Listening for connections\n");
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
            childpid=fork();
        
        if(childpid==-1){
            perror("fork");
        
                    //h logikh einai na einai kathe fora etoimo na lavei kainourio pid kai na trexei tautoxrona ta upoloipa
        }else if(childpid == 0){
            printf("Child process PID: %d\n",getpid());

            if (newsockfd < 0)
                error("ERROR on accept");
            if (inet_ntop(AF_INET, &cli_addr.sin_addr, str, INET_ADDRSTRLEN) == NULL) {
                fprintf(stderr, "Could not convert byte to address\n");
                exit(1);
            }
            printf("=====================\n");
            printf(" Accepted Connection \n");
            fprintf(stdout, "The client address is: %s\n", str);
            printf("=====================\n");
            grandchildpid=fork();
            if(grandchildpid==-1){
                perror("fork");
            }else if(grandchildpid==0){                                                   //The user can only type either a command with no parameter or a command with only one parameter(of course you can combine them)
                while(1){                                                                 //For example: ls -la      
                    bzero(buffer, 256);
                    n = read(newsockfd, buffer, 255);
                    if (n < 0) 
                        error("ERROR reading from socket");
                    
                    if(strcmp(buffer,"END\n")==0){
                        char text[3];
                        srand(getpid());
                        int random = rand()%20;
                        printf("Sending Random Number %d\n",random);
                        sprintf(text, "%d", random);
                        strcpy(buffer,text);
                        n = write(newsockfd,buffer, sizeof(buffer));
                        break;
                    }else{
                        printf("Executing Command for pid %d: %s",getpid(), buffer);
                        int newpid = fork();
                        if(newpid == -1){
                            perror("fork");
                        }else if(newpid == 0){
                            int i=0;
                            int z=0;
                            
                            
                            while(buffer[i]!='\n'){                 //counting the size
                                i++;
                            }
                            char tmp[i-1];
                            char tmp3[i-1];
                            i=0;
                            int cut=0; //0 false, 1 true
                            while(buffer[i]!='\n'){
                                if(buffer[i]==' '){ 
                                    cut=1;
                                    i++;
                                    continue;
                                }
                                if(cut==1){                         //creating the parameter after the 
                                    tmp[z]=buffer[i];
                                    z++;
                                }
                                i++;
                            }
                            tmp[z]='\0';
                            char *tmp2[2];
                            i=0;
                            while(buffer[i]!='\n' && buffer[i]!=' '){
                                tmp3[i]=buffer[i];
                                i++;
                            }
                            tmp3[i]='\0';
                            tmp2[0] = tmp3;
                            dup2(newsockfd,STDOUT_FILENO);
                            dup2(newsockfd,2);
                            int check;
                            if(strlen(tmp)==0){
                                check = execlp(tmp2[0],tmp2[0],NULL);
                            }else{
                                check = execlp(tmp2[0],tmp2[0],tmp,NULL);
                            }
                            if(check==-1){
                                perror("execlp");
                            }
                            bzero(buffer, 256);
                            buffer[255] = '\0';
                            exit(EXIT_SUCCESS);
                        }else{
                            int checkpid = wait(&status);
                            if(checkpid == -1){
                                perror("wait");
                            }else{
                                printf("Finished command\n");
                            }
                        }
                    }
                    
                }
            close(newsockfd);
            exit(EXIT_SUCCESS); //grandchild
            }else{                  //grandchild's father
               while(1){
                    int exit_grandchild = wait(&status);
                    if(exit_grandchild==-1){
                        break;
                    }else if(exit_grandchild==grandchildpid){
                        if(WIFEXITED(status)){
                            printf("====================\n");
                            printf("Process %d ended with termination code %d\n",grandchildpid,WEXITSTATUS(status));
                            printf("====================\n");
                        }
                    }  
                } 
            }
        }
    } 
    return 0;
}
