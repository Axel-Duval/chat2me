#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netdb.h>

/**
* This version allows a client to send a message
* Run the program : gcc -o client client.c -lpthread && ./client TARGET_IP PORT
*/

#define MAX_USERNAME_LENGTH 20
#define MAX_BUFFER_LENGTH 256
#define JOINER_LENGTH 4

void *sendMsg(void* dS){
    /* Get server's socket */
    int* arg = dS;
    char buffer[MAX_BUFFER_LENGTH];
    int sd;
    while(1){
        /* Clean the buffer */
        memset(buffer, 0, strlen(buffer));
        
        /* Get the message to send */
        fgets(buffer, MAX_BUFFER_LENGTH, stdin);

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
    char buffer[MAX_BUFFER_LENGTH + JOINER_LENGTH + MAX_USERNAME_LENGTH];
    int rc;
    while(1){
        /* Clean the buffer */
        memset(buffer, 0, strlen(buffer));

        /* Recieve the message */
        while(rc = recv(*arg, &buffer, sizeof(buffer), 0) <= 0){
           if(rc == 0){
                /* Connexion lost */
                break;
           }
        }
        /* Print the message */
        printf("%s\n", buffer);
    }
}


int main(int argc, char *argv[]){

    /* Checking args */
    if(argc != 3){
        printf("! I need to be call like -> :program SERVER_IP PORT !\n");
        exit(1);
    }

    /* Ask for username */
    char username[MAX_USERNAME_LENGTH];
    printf("Choose your username : ");
    fgets(username, MAX_USERNAME_LENGTH, stdin);

    /* Define target (ip:port) with calling program parameters */
    struct addrinfo hints, *res, *p;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // the version of ip addresses isn't specified
    hints.ai_socktype = SOCK_STREAM; //TCP

    /*Identify current address*/
    if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) { //recover the version of the given ip
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    exit(EXIT_FAILURE);
    }

    p = res;
    int dS;
      
    if (p->ai_family == AF_INET) { // IPv4
        printf("IPV4\n");

        /* Define target (ip:port) with calling program parameters */
        struct sockaddr_in aS;
        aS.sin_family = AF_INET;
        aS.sin_port = htons(atoi(argv[2]));

        inet_pton(AF_INET, argv[1], &(aS.sin_addr)); 
        socklen_t lgA = sizeof(struct sockaddr_in); 

        /* Create stream socket with IPv4 domain and IP protocol */
        dS = socket(PF_INET, SOCK_STREAM, 0);
        if (dS < 0){
            perror("! Issue whith socket creation !");
            return 1;
        }

        /* Open connexion */
        int con = connect(dS, (struct sockaddr *) &aS, lgA); 

        if (con<0){
            perror("! Can't find the target !");
            return 1;
        }
    }
    else { // IPv6
        printf("IPV6\n");

        /* Define target (ip:port) with calling program parameters */
        struct sockaddr_in6 aS;
        aS.sin6_family = AF_INET6;
        aS.sin6_port = htons(atoi(argv[2]));

        inet_pton(AF_INET6, argv[1], &(aS.sin6_addr)); 
        socklen_t lgA = sizeof(struct sockaddr_in6); 

        /* Create stream socket with IPv6 domain and IP protocol */
        dS = socket(PF_INET6, SOCK_STREAM, 0);
        if (dS < 0){
            perror("! Issue whith socket creation !");
            return 1;
        }

        /* Open connexion */
        int con = connect(dS, (struct sockaddr *) &aS, lgA); 

        if (con<0){
            perror("! Can't find the target !");
            return 1;
        }
    }

    /*Free memory*/
    freeaddrinfo(res); 

    /* Send username to the server */
    int sd;
    while(sd = send(dS,&username,strlen(username)-1,0) <= strlen(username)-2){
        if(sd == 0){
            /* Connexion lost */
            exit(1);
        }
    }
    
    /* Create 2 threads for recieve and send messages */
    pthread_t sdM;
    pthread_t rcM;
    pthread_create(&sdM,0,sendMsg,&dS);
    pthread_create(&rcM,0,recvMsg,&dS);

    /* Waiting for the end of communication to finish the program */
    pthread_join(sdM,0);  

    /* Close connexion */
    close(dS);
    return 1;
}