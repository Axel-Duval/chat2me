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
* Run the program : gcc -o client client.c && ./client TARGET_IP PORT -lpthread
*/

void *sendMsg(void* dS){
    /* Get server's socket */
    int* arg = dS;
    char buffer[256];
    int sd;
    while(1){

        /* Get the message to send */
        fgets(buffer,256,stdin);

        /* Check if it's the end of communication */
        char *chfin = strchr(buffer, '\n');
        *chfin = '\0';
        if(strcmp(buffer,"fin") == 0){
            printf("End of the communication ...\n");
            break;
        }

        /* Send the message */
        int sd;
        while(sd = send(*arg,&buffer,strlen(buffer),0) <= strlen(buffer)-1){
           if(sd == 0){
                /* Connexion lost */
                break;
           }
        }
    }
    pthread_exit(NULL);
}

void *recvMsg(void* dS){
    /* Get server's socket */
    int* arg = dS;
    char buffer[256];    
    int rc;
    while(1){

        /* Recieve the message */
        int rc;
        while(rc = recv(*arg, &buffer, sizeof(buffer), 0) <= 0){
           if(rc == 0){
                /* Connexion lost */
                break;
           }
        }

        /* Print the message */
        printf("- %s\n",buffer);
    }
}


int main(int argc, char *argv[]){

    /* Checking args */
    if(argc != 3){
        printf("! I need to be call like -> :program TARGET_IP PORT -lpthread !\n");
        exit(1);
    }

    /* Define target (ip:port) with calling program parameters */
    struct sockaddr_in aS ;
    aS.sin_family = AF_INET;
    aS.sin_port = htons(atoi(argv[2])) ;
    inet_pton(AF_INET,argv[1],&(aS.sin_addr));    
    socklen_t lgA = sizeof(struct sockaddr_in);

    /* Create stream socket with IPv4 domain and IP protocol */
    int dS= socket(PF_INET, SOCK_STREAM, 0);
    if(dS == -1){
		perror("! Issue whith socket creation !\n");
		exit(1);
	}

    /* Open connexion */
    int connexion = connect(dS, (struct sockaddr *) &aS, lgA);
    if(connexion < 0){
        perror("! Can't find the target !\n");
        exit(1);
    }

    
    /* Create 2 threads for recieve and send messages */
    pthread_t sdM;
    pthread_t rcM;
    pthread_create(&sdM,0,sendMsg,&dS);
    pthread_create(&rcM,0,recvMsg,&dS);

    /* Waiting for the end of communication (fin) to finish the program */
    pthread_join(sdM,0);  

    /* Close connexion */
    close(dS);
    return 1;
}