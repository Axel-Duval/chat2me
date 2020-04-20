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
    
    struct addrinfo hints, *res, *p;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // the version of ip addresses isn't specified
    hints.ai_socktype = SOCK_STREAM; //TCP

    if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) { //recover the version of the given ip
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    exit(EXIT_FAILURE);
    }

    p = res;
    int dS;
      
    // Identification de l'adresse courante
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

    // Libération de la mémoire occupée par les enregistrements
    freeaddrinfo(res);


    /* Waiting the message from the server */
    int numClient;
    int rc = recv(dS, &numClient, sizeof(numClient), 0);
    if (rc<0){
        perror ("! Problem with the reception of num client !");
        return 1;
    }

    /* client 1 */
    if(numClient == 1){
        printf("You are client n°1. Waiting for client 2... \n");
        //Attendre 2ème client
        char msgConfirmation[MAX_BUFFER_LENGTH];
        int rc = recv(dS, &msgConfirmation, MAX_BUFFER_LENGTH, 0);
        if (rc<0){
            perror("! Problem with the reception of the starting message !");
            return 1;
        } else {
            printf("%s\n", msgConfirmation);
        }

        /* Now they can communicate */

        while(1){
            
            /* client 1 sends a new message to the server */ 
            char buffer[MAX_BUFFER_LENGTH]; 
            printf("You : ");
            fgets(buffer,MAX_BUFFER_LENGTH,stdin);
            *strchr(buffer, '\n') = '\0';
            int sd = send(dS,&buffer,strlen(buffer),0);
            if (sd<0){
                perror("! Problem with the message sending !");
                return 1;
            }

            if(strcmp(buffer, "fin") == 0){
                printf("You completed the communication\n");
                close(dS);
                return 0;
            }

            /* client 1 receives the message from the server */
            char msg2[MAX_BUFFER_LENGTH];
            rc = recv(dS, &msg2, MAX_BUFFER_LENGTH, 0);
            if (rc<0){
                perror ("! Problem with the message reception !");
                return 1;
            }

            printf("Client 2 : %s\n", msg2);

            if(strcmp(msg2, "fin") == 0){
                printf("Client 2 completed the communication\n");
                close(dS);
                return 0;
            }
        }
        close(dS);
    } 

    /* client 2 */
    else if (numClient == 2){
        printf("%s\n", "You are client 2.");
        while(1){
            /* client 2 receives the message from the server */ 
            char msg1[MAX_BUFFER_LENGTH]; 
            int rc = recv(dS, &msg1, MAX_BUFFER_LENGTH, 0);

            if (rc<0){
                perror ("! Problem with the message reception !");
                return 1;
            }

            printf("Client 1 : %s\n", msg1);

            if(strcmp(msg1, "fin") == 0){
                printf("Communication fermée\n");
                close(dS);
                return 0;
            }

            /* client 2 sends a new message to the server */
            char buffer[MAX_BUFFER_LENGTH]; 
            printf("You : ");
            fgets(buffer,MAX_BUFFER_LENGTH,stdin);
            *strchr(buffer, '\n') = '\0';
            int sd = send(dS,&buffer,strlen(buffer),0);
            if (sd<0){
                perror("! Problem with the message sending !");
                return 1;
            }

            if(strcmp(buffer, "fin") == 0){
                printf("You completed the communication\n");
                close(dS);
                return 0;
            }
        }
        close(dS);
        
    }
    
return 0;
}