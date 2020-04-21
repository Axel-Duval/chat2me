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
* This version allows a server to recieve a message and print it
* Run the program : gcc -o server server.c && ./server PORT
*/

#define MAX_SOCKETS 10
#define MAX_USERNAME_LENGTH 20
#define MAX_BUFFER_LENGTH 256
#define JOINER_LENGTH 4
#define JOINER " - "

/* Global sockets array initialize with zeros */
int sockets[MAX_SOCKETS] = {0};

/* Create struct to share socket with username to threads */
struct sockets_struct
{
    char clientUsername[MAX_USERNAME_LENGTH];
    int socket;
};
struct sockets_struct clientStruct;


/* Print socket state */
void psockets(){
    printf("SOCKETS : ");
    for(int i = 0; i < MAX_SOCKETS; i++){
        printf("%d ",sockets[i]);
    }
    printf("\n");
}

/* Append new socket to sockets array */
int add_socket(int sockets[], int socket){
    int i = 0;
    while(i < MAX_SOCKETS && sockets[i] != 0){
        i++;
    }
    if(i == MAX_SOCKETS){
        /* End of array, no more space */
        perror("! Sockets array full !\n");
        return -1;
    }
    else{
        /* There is space for new socket */
        sockets[i] = socket;
        return socket;
    }
}

/* Remove socket to sockets array */
int remove_socket(int sockets[], int socket){
    int i = 0;
    while(i < MAX_SOCKETS && sockets[i] != socket){
        i++;
    }
    if(i == MAX_SOCKETS){
        /* End of array, socket not found */
        perror("! Can't find socket !\n");
        return -1;
    }
    else{
        /* Socket found, need to remove it */
        sockets[i] = 0;
        return 1;
    }
}

/* Create thread wich wait for client message and send it to all others clients */
void *thread_func(void *arg){

    /* Get client username and socket */
    struct sockets_struct *args = (void *)arg;
    char username[MAX_USERNAME_LENGTH];
    strcpy(username,args->clientUsername);
    int socketCli = args->socket;

    /* Create buffer for messages */
    char buffer[MAX_BUFFER_LENGTH];
    char joiner[JOINER_LENGTH];
    strcpy(joiner,JOINER);
    char message[MAX_BUFFER_LENGTH + 4 + MAX_USERNAME_LENGTH];

    /* Define some int */
    int rv;
    int sd;
    int i;

    while(1){
        /* Clean the buffer */
        memset(buffer, 0, sizeof(buffer));
        memset(message, 0, sizeof(message));

        /* Waiting for message from client */
        rv = recv(socketCli, &buffer, sizeof(buffer), 0);
        if(rv == 0){
            /* Connexion lost with client, need to remove his socket */
            remove_socket(sockets, socketCli);
            break;
        }
        else if(rv > 0){

            strcat(message,username);
            strcat(message,joiner);
            strcat(message,buffer);

            for(i = 0; i < MAX_SOCKETS; i++){
                /* Only send on valid sockets and not to our client socket... */
                if(sockets[i] != 0 && sockets[i] != socketCli){
                    /* Send message to client [i] */
                    while(sd = send(sockets[i], message, sizeof(message),0) <= 0){
                        /* Error sending message to client [i] */
                        if(sd == 0){
                            /* Because of connexion lost with client [i] need to remove his socket */
                            remove_socket(sockets, sockets[i]);
                            break;
                        }
                    }
                }
            }
            
        }
    }
    pthread_exit(NULL);
}
 

/* MAIN */
int main(int argc, char *argv[]){

    /* Checking args */
    if(argc != 2){
		perror("! I need to be call like -> :program PORT !\n");
		exit(1);
	}

    /* Define some variables */
    int tmp;
    char username[MAX_USERNAME_LENGTH];
    int rc;

    /* Define target (ip:port) with calling program parameters */
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int dS, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[MAX_BUFFER_LENGTH];
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = AF_WANPIPE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    s = getaddrinfo(NULL, argv[1], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
    Try each address until we successfully bind(2).
    If socket() (or bind()) fails, we (close the socket
    and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        dS = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (dS == -1){
            continue;
        }
        if (bind(dS, rp->ai_addr, rp->ai_addrlen) == 0){
            break;                  /* Success */
        }
        close(dS);
    }

    if (rp == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);
    printf("Socket created !\n");

    /* Starting the server */
    listen(dS,MAX_SOCKETS);
    printf("Start server on port : %s\n", argv[1]);

    while(1){

        /* Clean username buffer */
        memset(username, 0, sizeof(username));

        /* Create addr for a clients */
        struct sockaddr_in addrCli;
        socklen_t lg = sizeof(struct sockaddr_in);

        /* Accept the connexion from client */
        int socketCli = accept(dS, (struct sockaddr*)&addrCli,&lg);

        if(socketCli > 0){
            printf("\033[0;32mConnexion established with client : %s:%d \033[0m\n",inet_ntoa(addrCli.sin_addr),ntohs(addrCli.sin_port));

            /* Waiting for the username */
            while(rc = recv(socketCli, &username, sizeof(username), 0) < 0){
                /* Error getting username, retry to recieve */                
            }

            /* Add this new client to sockets tab */
            if(add_socket(sockets,socketCli) != -1){
                psockets();

                /* Bind socket and username to structure */
                strcpy(clientStruct.clientUsername,username); //:username (but username doesn't work...)
                clientStruct.socket = socketCli;

                /* Create thread */
                pthread_t thread;
                pthread_create(&thread, NULL, thread_func, (void *)&clientStruct);
            }
        }
    }

    /* Close connexion */
    close (dS);
    printf("Stop server\n");
    return 1;
}