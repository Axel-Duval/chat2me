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


/* Create struct to share sockets with threads */
struct sockets_struct
{
    int socket1;
    int socket2;
};
struct sockets_struct sockets;

/* Create thread 1 wich wait for client 1 message and send it to client 2 */
void *thread_1(void *arg){
    while(1){        
        /* Get sockets */
        struct sockets_struct *args = (void *)arg;
        int socket1 = args->socket1;
        int socket2 = args->socket2;

        /* Create buffer for messages */
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));

        /* Waiting for message from client 1 */
        int rv = recv(socket1, &buffer, sizeof(buffer), 0);
        if(rv == 0){
            /* Connexion lost with client 1 */
            break;    
        }
        else if(rv > 0){
            /* Message valid in the buffer */
            int sd;
            while(sd = send(socket2,&buffer, sizeof(buffer),0) <= 0){
                /* Error sending message to client 2 */
                if(sd == 0){
                    /* Because of connexion lost with client 2 */
                    break;
                }
            }
        }
    }
    pthread_exit(NULL);
}

/* Create thread 2 wich wait for client 2 message and send it to client 1 */
void *thread_2(void *arg){
    while(1){        
        /* Get sockets */
        struct sockets_struct *args = (void *)arg;
        int socket1 = args->socket1;
        int socket2 = args->socket2;

        /* Create buffer for messages */
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));

        /* Waiting for message from client 2 */
        int rv = recv(socket2, &buffer, sizeof(buffer), 0);
        if(rv == 0){
            /* Connexion lost with client 2 */
            break;    
        }
        else if(rv > 0){
            /* Message valid in the buffer */
            int sd;
            while(sd = send(socket1,&buffer, sizeof(buffer),0) <= 0){
                /* Error sending message to client 1 */
                if(sd == 0){
                    /* Because of connexion lost with client 1 */
                    break;
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
		printf("! I need to be call like -> :program PORT -lpthread !\n");
		exit(1);
	}

    /* Define some constants */
    int tmp;
    char confirm[20] = "You are connected...";
    char messageConfirmation[20] = "You can now chat !";

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

    /* Create addr for 2 clients */
    struct sockaddr_in addrCli1;
    struct sockaddr_in addrCli2;
    socklen_t lg1 = sizeof(struct sockaddr_in);
    socklen_t lg2 = sizeof(struct sockaddr_in);


    /* Accept the connexion from client 1 */
    int socketCli1;
    while(socketCli1 = accept(dS, (struct sockaddr*)&addrCli1,&lg1) < 0){
        /* Acceptation error */
    }
    printf("\033[0;32mConnexion established with our first client : %s:%d \033[0m\n",inet_ntoa(addrCli1.sin_addr),ntohs(addrCli1.sin_port));
    while(tmp = send(socketCli1,&confirm, sizeof(confirm),0) < 0){
        /* Error sending message to client 1 */
        if(tmp == 0){
            /* Because of connexion lost with client 1, need to stop the program */
            exit(1);
        }
    }

    /* Accept the connexion from client 2 */
    int socketCli2 = accept(dS, (struct sockaddr*)&addrCli2,&lg2);
    if(socketCli2 > 0){
        printf("\033[0;32mConnexion established with our second client : %s:%d \033[0m\n",inet_ntoa(addrCli2.sin_addr),ntohs(addrCli2.sin_port));
    }
    tmp = send(socketCli2,&confirm, sizeof(confirm),0);
    if(tmp < 0){
        printf("! Error sending confirmation to second client !\n");
    }

    /* Sending start signal to clients */
    while(tmp = send(socketCli1,&messageConfirmation, sizeof(messageConfirmation),0) < 0){
        printf("! Error sending chat start\n");
    }
    while(tmp = send(socketCli2,&messageConfirmation, sizeof(messageConfirmation),0) < 0){
        printf("! Error sending chat start\n");
    }

    /* Bind sockets on structure */
    sockets.socket1 = socketCli1;
    sockets.socket2 = socketCli2;

    /* Create 2 threads */
    pthread_t thread1;
    pthread_t thread2;
    pthread_create(&thread1, NULL, thread_1, &sockets);
    pthread_create(&thread2, NULL, thread_2, &sockets);

    /* Join threads */
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    /* Close connexion */
    close (dS);
    printf("Stop server\n");
    return 1;
}