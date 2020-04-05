#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
* This version allows a client to send a message to another client
* Run the program : gcc -o client client.c && ./client TARGET_IP PORT
*/

#define MAX_BUFFER_LENGTH 256

int main(int argc, char *argv[])
{

    /* Checking args */
    if (argc != 3){
        printf("! I need to be call like : program TARGET_IP PORT !\n");
        exit(1);
    }

    /* Create buffer for initialization messages */
    char buffer[MAX_BUFFER_LENGTH];
    memset(buffer, 0, strlen(buffer));

    /* Define target (ip:port) with calling program parameters */
    struct sockaddr_in aS;
    aS.sin_family = AF_INET;
    aS.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &(aS.sin_addr));
    socklen_t lgA = sizeof(struct sockaddr_in);

    /* Create stream socket with IPv4 domain and IP protocol */
    int dS = socket(PF_INET, SOCK_STREAM, 0);
    if(dS == -1){
		perror("! Issue whith socket creation !\n");
		exit(1);
	}

    /* Open connexion */
    int con = connect(dS, (struct sockaddr *)&aS, lgA);
    if (con < 0)
    {
        perror("! Can't find the target !\n");
        exit(1);
    }

    /* Waiting the message from the server */
    int numClient;
    int rc = recv(dS, &buffer, sizeof(buffer), 0);
    if (rc < 0){
        perror("! Problem with the reception of num client !");
    }
    else{
        numClient = atoi(buffer);
        printf("You are client n°%d\n", numClient);
        memset(buffer, 0, strlen(buffer));

        /* Blocking function until the 2nd connection */
        rc = recv(dS, &buffer, sizeof(buffer), 0);
        if (rc < 0){
            perror("! Problem with the reception of the starting message !");
        }
        else{
            printf("%s\n", buffer);
            memset(buffer, 0, strlen(buffer));
        }
    }

    /* Now they can communicate */

    /* Define some variables */
    char msg1[MAX_BUFFER_LENGTH];
    char msg2[MAX_BUFFER_LENGTH];
    int rcv;
    int sd;
    int cmp;

    /* client 1 */
    if (numClient == 1){
        while (1){

            /* client 1 sends a new message to the server */
            printf("\nYou : ");
            fgets(msg1, MAX_BUFFER_LENGTH, stdin);
            char *chfin = strchr(msg1, '\n');
            *chfin = '\0';
            sd = send(dS, msg1, strlen(msg1), 0);
            if (sd < 0){
                perror("! Problem with the message sending !");
            }
            else{
                cmp = strcmp(msg1, "fin");
                if (cmp == 0){
                    printf("\nYou completed the communication\n");
                    break;
                }
                memset(msg1, 0, strlen(msg1));
            }

            /* client 1 receives the message from the server */
            rcv = recv(dS, &msg2, sizeof(msg2), 0);
            if (rcv < 0){
                perror("! Problem with the message reception !");
            }
            else{
                cmp = strcmp(msg2, "fin");
                if (cmp == 0)
                {
                    printf("\nClient 2 completed the communication\n");
                    break;
                }
                printf("\nClient 2: %s", msg2);
                memset(msg2, 0, strlen(msg2));
            }
        }
    }
    /* client 2 */
    else{
        while (1){
            /* client 2 receives the message from the server */
            rcv = recv(dS, &msg1, sizeof(msg1), 0);
            if (rcv < 0){
                perror("! Problem with the message reception !");
            }
            else{
                cmp = strcmp(msg1, "fin");
                if (cmp == 0)
                {
                    printf("\nClient 1 completed the communication\n");
                    break;
                }
                printf("\nClient 1 : %s", msg1);
                memset(msg1, 0, strlen(msg1));
            }

            /* client 2 sends a new message to the server */
            printf("\nYou : ");
            fgets(msg2, MAX_BUFFER_LENGTH, stdin);
            char *chfin = strchr(msg2, '\n');
            *chfin = '\0';
            sd = send(dS, msg2, strlen(msg2), 0);
            if (sd < 0){
                perror("! Problem with the message sending !");
            }
            else{
                cmp = strcmp(msg2, "fin");
                if (cmp == 0){
                    printf("\nYou completed the communication\n");
                    break;
                }
                memset(msg2, 0, strlen(msg2));
            }
        }
    }
    
    /* Close connexion */
    close(dS);
    return 1;
}