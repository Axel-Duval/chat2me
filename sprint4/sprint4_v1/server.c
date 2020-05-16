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
#define MAX_NAME_LENGTH 20
#define MAX_BUFFER_LENGTH 356
#define JOINER_LENGTH 4
#define JOINER " - "
#define FILE_PROTOCOL_LENGTH 10
#define FILE_PROTOCOL "-FILE-"
#define MAX_CHANNELS 5
#define MAX_CLIENTS_BY_CHANNEL 5

/* Global sockets array initialize with zeros */
int sockets[MAX_SOCKETS] = {0};
int choiceChannel[MAX_SOCKETS] = {-1};

/* Create struct to share socket with username to threads */
struct sockets_struct
{
    char clientUsername[MAX_NAME_LENGTH];
    int numConnectedChannel;
    int socket;
};
struct sockets_struct clientStruct;

struct channel_struct{
    int numChannel;
	char name[MAX_NAME_LENGTH];
	char description[MAX_BUFFER_LENGTH];
	int nbClientConnected;
};
struct channel_struct channels[MAX_CHANNELS];

/* Print socket state */
void psockets(){
    printf("SOCKETS : ");
    for(int i = 0; i < MAX_SOCKETS; i++){
        printf("%d ",sockets[i]);
    }
    printf("\n");
}

/* Append new socket to sockets array */
int add_socket(int sockets[], int socket, int chosenCh){
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
        choiceChannel[i]=chosenCh;
        return i; //return index of the socket;
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
        int clientCh;
        clientCh = choiceChannel[i];
        channels[clientCh].nbClientConnected -= 1;
        choiceChannel[i]=-1;
        return 1;
    }
}

void init_channels(){
    for (int i = 0; i < MAX_CHANNELS; i++){
        channels[i].numChannel=i;
        channels[i].nbClientConnected = 0;
        switch (i) {
            case 0:
                strcpy(channels[i].name,"Channel 1");
                strcpy(channels[i].description,"Join to speak about Java Project");
              break;
        
            case 1:
                strcpy(channels[i].name,"Channel 2");
                strcpy(channels[i].description,"Join to speak about FAR Project");
              break;
              
            case 2:
                strcpy(channels[i].name,"Channel 3");
                strcpy(channels[i].description,"Join to speak about Swift Project");
              break;
              
            case 3:
                strcpy(channels[i].name,"Channel 4");
                strcpy(channels[i].description,"Join to speak about JS Project");
              break;
              
            default:
              strcpy(channels[i].name,"Random");
              strcpy(channels[i].description,"Join to speak about everything");
        }
    }
}

/* Check if the given channel exists or isn't full
    Return its number if all it's ok, -1 otherwise */
int check_channel(char name[]){
    /* remove the last character */
    char *chname = strchr(name, '\n');
    *chname = '\0';

    for(int i = 0; i < MAX_CHANNELS; i++){
        if(strcmp(channels[i].name,name) == 0 && channels[i].nbClientConnected < MAX_CLIENTS_BY_CHANNEL){
            return channels[i].numChannel;
        }
    }
    return -1;
}

/* Create thread which wait for client message and send it to all others clients */
void *thread_func(void *arg){

    /* Get client username and socket */
    struct sockets_struct *args = (void *)arg;
    char username[MAX_NAME_LENGTH];
    strcpy(username,args->clientUsername);
    int socketCli = args->socket;
    int numChannel = args->numConnectedChannel;

    /* Create buffer for messages */
    char buffer[MAX_BUFFER_LENGTH];
    char joiner[JOINER_LENGTH];
    strcpy(joiner,JOINER);
    char message[MAX_BUFFER_LENGTH + 4 + MAX_NAME_LENGTH];
    char file_protocol[FILE_PROTOCOL_LENGTH] = FILE_PROTOCOL;

    /* Define some int */
    int rv;
    int sd;
    int i;
    int is_file = 0;
    int flip_flop = 0;

    while(1){
        /* Clean the buffer */
        memset(buffer, 0, sizeof(buffer));
        flip_flop = 0;

        /* Waiting for message from client */
        rv = recv(socketCli, &buffer, sizeof(buffer), 0);
        if(rv == 0){
            /* Connexion lost with client, need to remove his socket */
            remove_socket(sockets, socketCli);
            break;
        }
        else{

            if(strcmp(buffer,"fin") == 0){
                //Disconnect client.
                remove_socket(sockets, socketCli);
                printf("Client disconnected ...\n");
                break;
            }

            /* Switcher */
            if(strcmp(buffer, file_protocol) == 0){
                printf("File protocol detected !\n");
                if(is_file == 0){
                    is_file = 1;
                }
                else{
                    is_file = 0;
                    /* Flip flop */
                    flip_flop = 1;
                }
            }

            /* Check if it's a simple text message to format the sending message */
            if(flip_flop == 0 && is_file == 0){

                memset(message, 0, sizeof(message));
                strcat(message,username);
                strcat(message,joiner);
                strcat(message,buffer);

                /* Add here send messsage to only sockets in the channel */
                for(i = 0; i < MAX_SOCKETS; i++){
                    /* Only send on valid sockets and not to our client socket... */
                    if(sockets[i] != 0 && sockets[i] != socketCli && choiceChannel[i] == numChannel){ //
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
            /* Check if it's a file */
            else if(is_file == 1){
                for(i = 0; i < MAX_SOCKETS; i++){
                    /* Only send on valid sockets and not to our client socket... */
                    if(sockets[i] != 0 && sockets[i] != socketCli && choiceChannel[i] == numChannel){
                        /* Send message to client [i] */
                        while(sd = send(sockets[i], buffer, sizeof(buffer),0) <= 0){
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
    char username[MAX_NAME_LENGTH];
    int rc,sd;

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

    /* Starting the server */
    listen(dS,MAX_SOCKETS);
    printf("Start server on port : %s\n", argv[1]);

    init_channels();

    while(1){

        /* Clean username buffer */
        memset(username, 0, sizeof(username));

        /* Create addr for a clients */
        struct sockaddr_in addrCli;
        socklen_t lg = sizeof(struct sockaddr_in);

        /* Accept the connexion from client */
        int socketCli = accept(dS, (struct sockaddr*)&addrCli,&lg);

        /* Creation of the channels list */
        char message[MAX_BUFFER_LENGTH]="";
        strcat(message,"\n\n");
        strcat(message,"------ Channel list ------");
        strcat(message,"\n");
        for(int i = 0; i < MAX_CHANNELS; i++){
            char nbC[2];
            int nbCoInt = channels[i].nbClientConnected;
            sprintf(nbC,"%d",nbCoInt);
            char nbMax[2];
            sprintf(nbMax,"%d",MAX_CLIENTS_BY_CHANNEL);

            strcat(message,channels[i].name);
            strcat(message," : ");

            if(nbCoInt < MAX_CLIENTS_BY_CHANNEL-2){
            //green : the channel isn't full
                char nbCh[] = "\033[00;32m[";
                strcat(nbCh,nbC);
                strcat(nbCh,"/");
                strcat(nbCh,nbMax);
                strcat(nbCh,"]\033[0m");

                strcat(message,nbCh);

            } else if(nbCoInt >= MAX_CLIENTS_BY_CHANNEL-2 && nbCoInt < MAX_CLIENTS_BY_CHANNEL){
            //orange : only a few places (2)
                char nbCh[] = "\033[0;33m[";
                strcat(nbCh,nbC);
                strcat(nbCh,"/");
                strcat(nbCh,nbMax);
                strcat(nbCh,"]\033[0m");

                strcat(message,nbCh);

            } else {
            //red : the channel is full
                char nbCh[] = "\033[0;31m[";
                strcat(nbCh,nbC);
                strcat(nbCh,"/");
                strcat(nbCh,nbMax);
                strcat(nbCh,"]\033[0m");

                strcat(message,nbCh);

            }

            strcat(message,"\n");
            strcat(message,channels[i].description);
            strcat(message,"\n\n");
        }

        if(socketCli > 0){
            printf("\033[0;32mConnexion established with client : %s:%d \033[0m\n",inet_ntoa(addrCli.sin_addr),ntohs(addrCli.sin_port));

            /* Waiting for the username */
            rc = recv(socketCli, &username, sizeof(username), 0);
            if (rc< 0){
                printf("! Error receiving username from client !\n");
            }

            /* Send the channel list */
            sd = send(socketCli, &message, sizeof(message),0);
            if(sd < 0){
                printf("! Error sending the channel list to client !\n");
            }

            /* Receive the chosen channel name */
            char chosenChannel[MAX_NAME_LENGTH];
            rc = recv(socketCli, &chosenChannel, sizeof(chosenChannel),0);
            if(rc <0){
                printf("! Error receiving chosen channel from client !\n");
            }

            int chosenCh;
            chosenCh = check_channel(chosenChannel);
            /* Check if the client can be connected to the chosen channel */
            while(chosenCh < 0){
                memset(chosenChannel, 0, MAX_NAME_LENGTH);
                /* Send the error message */
                sd = send(socketCli, &chosenCh, sizeof(chosenCh),0);
                if(sd < 0){
                  printf("! Error sending the error channel message to client !\n");
                }

                rc = recv(socketCli, &chosenChannel, sizeof(chosenChannel), 0);
                if(rc < 0){
                    printf("! Error receiving new chosen channel from client !\n");
                }

                chosenCh = check_channel(chosenChannel);

            }

            /* Send the num channel */
            sd = send(socketCli, &chosenCh, sizeof(chosenCh),0);
            if(sd < 0){
              printf("! Error sending the error channel message to client !\n");
            }

            /* The channel is available (id in chosenCh) */
            printf("Client enter in the channel\n");
            channels[chosenCh].nbClientConnected+=1;

            /* Add this new client to sockets tab */
            int indexNewSocket;
            if(indexNewSocket = add_socket(sockets,socketCli,chosenCh) != -1){
                psockets();

                /* Bind socket and username to structure */
                strcpy(clientStruct.clientUsername,username); //:username (but username doesn't work...)
                clientStruct.numConnectedChannel=chosenCh; //the num of the channel he's connected
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