#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
* This version allows a server to recieve a message and print it
* Run the program : gcc -o server server.c && ./server PORT
*/

#define MAX_SOCKETS 10
#define MAX_BUFFER_LENGTH 256

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

    /* Define target (ip:port) with calling program parameters */
    struct sockaddr_in ad;
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(atoi(argv[1]));

    /* Create stream socket with IPv4 domain and IP protocol */
    int dS = socket(PF_INET, SOCK_STREAM, 0);
    if (dS == -1)
    {
        printf("! Issue whith socket creation !\n");
        exit(1);
    }

    /* Binding the sockets */
    bind(dS, (struct sockaddr *)&ad, sizeof(ad));

    /* Starting the server */
    listen(dS, MAX_SOCKETS);
    printf("Start server on port : %s\n", argv[1]);

    /* Create address for clients */
    struct sockaddr_in addrCli1;
    struct sockaddr_in addrCli2;
    socklen_t lg1 = sizeof(struct sockaddr_in);
    socklen_t lg2 = sizeof(struct sockaddr_in);

    /* Define some variables */
    int socketCli1;
    int socketCli2;
    char *numClient;
    int numC;
    char messageConfirmation[19] = "You can chat now !";

    while (1){
        
        /* socketCli1 */
        socketCli1 = accept(dS, (struct sockaddr *)&addrCli1, &lg1);
        if (socketCli1 < 0){
            perror("! Error socketCli1 creation !");
        }
        printf("\n\033[0;32mConnexion established with our first client\033[0m");

        /* socketCli2 */
        socketCli2 = accept(dS, (struct sockaddr *)&addrCli2, &lg2);
        if (socketCli2 < 0){
            perror("! Error socketCli1 creation !");
        }
        printf("\n\033[0;32mConnexion established with our second client\033[0m");

        /* Sending the number to the client 1*/
        numClient = "1";
        numC = send(socketCli1, numClient, sizeof(char), 0);
        if (numC < 0){
            printf("! Error sending the number to first client !\n");
        }        

        /* Sending the number to the client 2*/
        numClient = "2";
        numC = send(socketCli2, numClient, sizeof(char), 0);
        if (numC < 0){
            printf("! Error sending the number to first client !\n");
        }

        /* Sending start signal to chat */        
        while (numC = send(socketCli1, &messageConfirmation, sizeof(messageConfirmation), 0) < 0){
            printf("! Error sending chat start !\n");
        }
        while (numC = send(socketCli2, &messageConfirmation, sizeof(messageConfirmation), 0) < 0){
            printf("! Error sending chat start !\n");
        }

        /* Conversation starts */

        /* Define some variables */
        char msg1[MAX_BUFFER_LENGTH];
        char msg2[MAX_BUFFER_LENGTH];
        int rcv;
        int sd ;
        int cmp;

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