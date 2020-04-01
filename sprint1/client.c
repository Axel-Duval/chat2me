#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
* This version allows a client to send a message
* Run the program : gcc -o client client.c && ./client TARGET_IP PORT
*/

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
    connect(dS, (struct sockaddr *) &aS, lgA);
    printf("Connexion established !\n");

    /* Ask and send the message */
    printf("Enter your message : ");
    scanf("%s",buffer);
    send(dS, buffer, strlen(buffer), 0) ; 

    /* Close connexion */
    close(dS) ;

    return 1;
}