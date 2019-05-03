#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <signal.h>
int sockfdglobal;
void error(const char *msg)
{
    perror(msg);
    exit(0);
}


void signalhandler(int signum){                 //Signal handler when the client press CTRL+C
    int n;
    char buffer[256];
     bzero(buffer, 256);
    printf("\nCaught Signal %d. Terminating Connection\n",signum);
    strcpy(buffer,"END\n");
   n = write(sockfdglobal, buffer, strlen(buffer));
        if (n < 0) error("ERROR writing to socket");
    close(sockfdglobal);
    exit(signum);
    
}

void game(int turns, int serverandom){                  //Thats the function of the game
    int numbers[turns];
    int i;
    int option[turns];
    int check;
    int z=0;
    printf("You have %d tries in order to win the server. Enter between 0-20 in order to win\n", turns);
    for(i=0;i<turns;i++){
        printf("(%d): ",i+1);
        check = scanf("%d",&option[i]);
        if(check!=1){                           //If the user enter a char then the game ends
            printf("Hmmm, it seems like you're trying to find a bug here..That wasn't nice.I don't want to play anymore!!\n");
            return;
        }
        if(option[i]<0 || option[i]>20){        //if the user enters an invalid option, then he/she loses that turn
            printf("It seems like you tried cheating, so you will have a penalty!!You missed your chance!!\n");
            option[i]==-1;
            continue;
        }
        numbers[i]=option[i];
        
        
    }
    
    for(i=0;i<turns;i++){
        if(numbers[i]==serverandom){
            printf("YOU WON!! SERVER PICKED %d!! BYE BYE\n",serverandom);
            return;
        }
                
    }
    printf("YOU LOST!! SERVER PICKED %d. TRY SOME OTHER TIME..BYE BYE\n",serverandom);
    
}


//parent process -> checking for new connections
//child process -> connecting
//grandchild process -> execute commands
int main(int argc, char *argv[])
{
    signal(SIGINT,signalhandler);
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int turns=0;
    char buffer[256];
    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (sockfd < 0)
        error("ERROR opening socket");
    sockfdglobal = sockfd;
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
            (char *)&serv_addr.sin_addr.s_addr,
            server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    while(1){
    printf("%s_> ",argv[1]);            
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);
    char exitkey[] = "END\n";
     if(strcmp(buffer,exitkey)==0){
         
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0) error("ERROR writing to socket");
        n = read(sockfd, buffer,strlen(buffer));
        if (n < 0) error("ERROR reading socket");
        close(sockfd);
        
        game(turns,atoi(buffer));
        return 0;
    }
    n = write(sockfd, buffer, strlen(buffer));
    turns++;
    if (n < 0)
        error("ERROR writing to socket");
    int count;
        do{
        bzero(buffer, 256);
        n = read(sockfd, buffer, 255);
        if (n < 0)
            error("ERROR reading from socket");
        printf("%s", buffer);
        ioctl(sockfd, SIOCINQ, &count);
        }while(count!=0);
        bzero(buffer,256);
    }
    close(sockfd);
    return 0;
}
