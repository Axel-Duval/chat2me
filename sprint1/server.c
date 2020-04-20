#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

/**
* This version allows a server to recieve a message and print it
* Run the program : gcc -o server server.c && ./server PORT
*/

#define MAX_SOCKETS 10
#define MAX_BUFFER_LENGTH 256
#define MAX_USERS 2

int nbUsersConnected = 0;
int users[MAX_USERS];

void connectUsers(int dS) {

    int socketCli;
    int sd;

    /* Starting the server */
    listen(dS, MAX_SOCKETS);
    printf("Starting server ... \n");

    while(nbUsersConnected < MAX_USERS) {

        /* Create address for clients */
        struct sockaddr_in addrCli;
        socklen_t lg = sizeof(struct sockaddr_in);
        
        socketCli = accept(dS, (struct sockaddr *)&addrCli, &lg);
        if (socketCli < 0){
            perror("! Error socketCli creation !");
        }

        else {
            users[nbUsersConnected] = socketCli;
            nbUsersConnected++;
            printf("\033[0;32mConnexion established with client %d \n\033[0m", nbUsersConnected);

            /* Sending the number to the client 1*/
            sd = send(socketCli, &nbUsersConnected, sizeof(int), 0);
            if (sd < 0){
                printf("! Error sending the number to client !\n");
            }
        }
    }

}

int main(int argc, char *argv[])
{

    /* Checking args */
    if (argc != 2){
        printf("! I need to be call like : program PORT !\n");
        exit(1);
    }

    /* Create buffer for messages */
    char buffer[MAX_BUFFER_LENGTH];
    memset(buffer, 0, strlen(buffer));

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
    freeaddrinfo(result);           /* No longer needed */
    /* Read datagrams and echo them back to sender */
    printf("Socket created ! \n");


    /* Define some variables */
    
    char messageConfirmation[19] = "You can chat now !";

    /* Define some variables */
    char msg1[MAX_BUFFER_LENGTH];
    char msg2[MAX_BUFFER_LENGTH];
    memset(msg1, 0, MAX_BUFFER_LENGTH);
    memset(msg2, 0, MAX_BUFFER_LENGTH);
    int rcv;
    int sd ;
    int cmp;

    while (1){

        connectUsers(dS);

        int socketCli1 = users[0];
        int socketCli2 = users[1];

        /* Sending start signal to chat */        
        sd = send(socketCli1, &messageConfirmation, sizeof(messageConfirmation), 0);
        if(sd < 0){
            perror("! Error sending chat start !");
        }


        /* Conversation starts */

        while (1){
            
            /* waiting for a message from the client 1 */
            rcv = recv(socketCli1, msg1, sizeof(msg1), 0);
            if (rcv < 0){
                perror("! Problem with the message reception !");
            }
            else{
                /* transmission to the client 2 */
                sd = send(socketCli2, &msg1, sizeof(msg1), 0);
                if (sd < 0){
                    perror("Problem with the message sending");
                }
                else{
                    cmp = strcmp(msg1, "fin");
                    if (cmp == 0){
                        /* Client 1 completed the communication */
                        nbUsersConnected--;
                        break;
                    }
                    memset(msg1, 0, strlen(msg1));
                }
            }

            /* waiting for a message from the client 2 */
            rcv = recv(socketCli2, msg2, sizeof(msg2), 0);
            if (rcv < 0){
                perror("! Problem with the message reception !");
            }
            else{
                /* transmission to the client 1*/
                sd = send(socketCli1, &msg2, sizeof(msg2), 0);
                if (sd < 0){
                    perror("Problem with the message sending");
                }
                else{
                    int cmp = strcmp(msg2, "fin");
                    if (cmp == 0){
                        /* Client 2 completed the communication */
                        nbUsersConnected--;
                        break;
                    }
                    memset(msg2, 0, strlen(msg2));
                }
            }
        }
    }

    /* Close connexion */
    close(dS);
    printf("Stop server\n");

    return 1;
}