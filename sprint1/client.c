#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

/**
* This version allows a client to send a message
* Run the program : gcc -o client client.c && ./client TARGET_IP PORT
*/

void *sendMsg(int dS){
    char buffer[256];
    int sd;
    while(1){
        printf("Send your message :\n");
        fgets(buffer,256,stdin);
        sd=send(dS,&buffer,strlen(buffer),0);
        if(sd<0){
            perror("Error to send the message.");
            exit(0);
        } 
        if(strcmp(buffer,"fin") == 0){
            printf("End of the communication ...\n");
            close(dS);
            exit(0);
        }
    }
}

void *recvMsg(int dS){
    char buffer[256];
    int rc;
    while(1){
        rc = recv(dS, &buffer, 256, 0);
        if(rc < 0){
            perror("Error to receive the message.");
            exit(0);
        }
        printf("Message receive : %s\n",buffer);
        if(strcmp(buffer,"fin") == 0){
            printf("End of the communication ...\n");
            close(dS);
            exit(0);
        }
    }
}


int main(int argc, char *argv[]){

    /* Checking args */
    if(argc != 3){
        printf("! I need to be call like : program TARGET_IP PORT !\n");
        exit(1);
    }

    /* Create buffer for messages */
    char buffer[256];
    memset(buffer,0,sizeof(char)*256);

    /* Define target (ip:port) with calling program parameters */
    struct sockaddr_in aS ;
    aS.sin_family = AF_INET;
    aS.sin_port = htons(atoi(argv[2])) ;
    inet_pton(AF_INET,argv[1],&(aS.sin_addr));    
    socklen_t lgA = sizeof(struct sockaddr_in);

    /* Create stream socket with IPv4 domain and IP protocol */
    int dS= socket(PF_INET, SOCK_STREAM, 0);
    printf("Socket created !\n");

    /* Open connexion */
    int con = connect(dS, (struct sockaddr *) &aS, lgA);
    if(con < 0){
        printf("! Can't find the target !\n");
        exit(1);
    }

    /* Waiting the message from the server */
    int rc = recv(dS, &buffer, sizeof(buffer),0);
    if(rc<0){
        perror("Problem with the reception");
    } else { 
        printf("%s\n", buffer);
        memset(buffer,0,sizeof(char)*256);

        /*Blocking function until the 2nd connection*/
        rc = recv(dS, &buffer, sizeof(buffer),0);
        if (rc<0) {
            perror("Problem with the reception");
        } else {
            printf("%s\n", buffer);
            memset(buffer,0,sizeof(char)*256);
        }
    }

    /*Now they can communicate*/
    

    pthread_t sdM;
    pthread_t rcM;       

    pthread_create(&sdM,0,sendMsg,dS);
    pthread_create(&rcM,0,recvMsg,dS);

    pthread_join(sdM,0);
    pthread_join(rcM,0);
    
   

    /* Close connexion */
    close(dS);

    return 1;
}