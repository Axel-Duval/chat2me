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
* This version allows a server to recieve a message and print it
* Run the program : gcc -o server server.c && ./server PORT
*/

struct sockets_struct
{
    int socket1;
    int socket2;
};

struct sockets_struct sockets;

void *thread_1(void *arg){
    struct sockets_struct *args = (void *)arg;
    int socket1 = args->socket1;
    int socket2 = args->socket2;
    int rcv = recv(socket1);
    pthread_exit(NULL);
}

void *thread_2(void *arg){
    struct sockets_struct *args = (void *)arg;
    int socket1 = args->socket1;
    int socket2 = args->socket2;
    printf("Nous sommes dans le thread 1.\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){

    /* Checking args */
    if(argc != 2){
		printf("! I need to be call like : program PORT !\n");
		exit(1);
	}

    /* Create buffer for messages */
    char buffer[256];
    memset(buffer,0,sizeof(buffer));

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
    listen(dS,10);
    printf("Start server on port : %s\n", argv[1]);

    /* Create addr for clients */
    struct sockaddr_in addrCli1;
    struct sockaddr_in addrCli2;
    socklen_t lg1 = sizeof(struct sockaddr_in);
    socklen_t lg2 = sizeof(struct sockaddr_in);

    int dSC;
    int tmp;
    int confirm = 1;
    while(1){
        /* Accept the connexion from client 1 */
        int socketCli1 = accept(dS, (struct sockaddr*)&addrCli1,&lg1);
        if(socketCli1 > 0){
            printf("\033[0;32m");
            printf("Connexion established with our first client : %s:%d\n",inet_ntoa(addrCli1.sin_addr),ntohs(addrCli1.sin_port));
            printf("\033[0m");
        }
        tmp = send(socketCli1,&confirm, sizeof(int),0);
        if(tmp < 0){
            printf("! Error sending confirmation to first client !\n");
        }

        /* Accept the connexion from client 2 */
        int socketCli2 = accept(dS, (struct sockaddr*)&addrCli2,&lg2);
        if(socketCli2 > 0){
            printf("\033[0;32m");
            printf("Connexion established with our second client : %s:%d\n",inet_ntoa(addrCli2.sin_addr),ntohs(addrCli2.sin_port));
            printf("\033[0m");
        }
        tmp = send(socketCli1,&confirm, sizeof(int),0);
        if(tmp < 0){
            printf("! Error sending confirmation to second client !\n");
        }

        /* Sending start signal to chat */
        char messageConfirmation[50] = "You can now chat !";
        while(tmp = send(socketCli1,&confirm, sizeof(messageConfirmation),0) < 0){
            printf("! Error sending chat start\n");
        }
        while(tmp = send(socketCli2,&confirm, sizeof(messageConfirmation),0) < 0){
            printf("! Error sending chat start\n");
        }

        /* Chat loop */
        sockets.socket1 = socketCli1;
        sockets.socket2 = socketCli2;
        pthread_t thread1;
        pthread_t thread2;
        printf("Avant la création du thread.\n");

        if (pthread_create(&thread1, NULL, thread_1, &sockets)) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }
        if (pthread_create(&thread2, NULL, thread_2, &sockets)) {
            perror("pthread_create");
            return EXIT_FAILURE;
        }

        if (pthread_join(thread1, NULL)) {
            perror("pthread_join");
            return EXIT_FAILURE;
        }
        if (pthread_join(thread2, NULL)) {
            perror("pthread_join");
            return EXIT_FAILURE;
        }

        printf("Après la création du thread.\n");
	}

    /* Close connexion */
    close (dS);
    printf("Stop server\n");

    return 1;
}