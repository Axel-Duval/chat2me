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

int main(int argc, char *argv[]){

    /* Checking args */
    if(argc != 2){
		printf("! I need to be call like : program PORT !\n");
		exit(1);
	}

    /* Create buffer for messages */
    char buffer [256];
    memset(buffer,0,sizeof(char)*256);

    /* Define target (ip:port) with calling program parameters */
    struct sockaddr_in ad;
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = htons(atoi(argv[1]));

    /* Create stream socket with IPv4 domain and IP protocol */
	int dS = socket(PF_INET,SOCK_STREAM,0);
    if(dS == -1){
		printf("! Issue whith socket creation !\n");
		exit(1);
	}
    printf("Socket created !\n");

    /* Binding the socket */
    bind(dS,(struct sockaddr*)&ad,sizeof(ad));

    /* Starting the server */
    listen(dS,5);
    printf("Start server on port : %s\n", argv[1]);

    /* Creation d'un buffer pour stocker les messages */
    struct sockaddr_in aC;
    socklen_t lg = sizeof(struct sockaddr_in);

    /* Accept the connexion from clients */
    int dSC= accept(dS, (struct sockaddr*)&aC,&lg);
    if(dSC > 0){
        printf("\033[0;32m");
        printf("Connexion established with : %s:%d\n",inet_ntoa(aC.sin_addr),ntohs(aC.sin_port));
        printf("\033[0m");
    }

    /*  */
    int rc;
    while(rc = recv(dSC, buffer, sizeof(buffer), 0) >= 0){
		/* Print the message*/
		if(rc == 0){
			printf("Got an empty message...");
		}
		else{
            printf("\033[0;33m");
			printf("%s:%d - %s",inet_ntoa(aC.sin_addr),ntohs(aC.sin_port),buffer);
            printf("\033[0m");
		}
		memset(buffer,0,sizeof(char)*256);
        break;
	}

    /* Close connexion */
    close (dS);
    printf("Stop server\n");

    return 1;
}