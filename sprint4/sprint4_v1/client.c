#include <dirent.h> 
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <netdb.h>

/**
* This version allows a client to send messages and files
* Run the program : gcc -o client client.c -lpthread && ./client TARGET_IP PORT
*/

#define MAX_NAME_LENGTH 20
#define MAX_BUFFER_LENGTH 356
#define JOINER_LENGTH 4
#define FILE_PROTOCOL_LENGTH 10
#define FILE_PROTOCOL "-FILE-"

/* Frequency for sending file chunks => 200Hz */
#define FREQUENCY 50000000L


void *sendFile(void* dS){
    /* Get server's socket and declare some variables */
    int* arg = dS;
    char buffer[MAX_BUFFER_LENGTH];
    int sd;
    char filename[MAX_BUFFER_LENGTH];
    char file_protocol[FILE_PROTOCOL_LENGTH] = FILE_PROTOCOL;
    DIR* directory;
	FILE* file = NULL;    

    /* Clear buffers */
    memset(filename, 0, MAX_BUFFER_LENGTH);
    memset(buffer, 0, MAX_BUFFER_LENGTH);

    /* For the timer */
    struct timespec tim, tim2;
    tim.tv_sec = 0;
    tim.tv_nsec = FREQUENCY;

    /* Print files in the current directory */
    directory = opendir (".");
    if (directory != NULL){
        struct dirent * dir_f;
        printf("---------------------FILES-----------------------\n");
        while ((dir_f = readdir(directory)) != NULL){					
            if(strcmp(dir_f->d_name,".")!=0 && strcmp(dir_f->d_name,"..")!=0){
                printf("%s\n", dir_f->d_name);
            }
        }
        closedir(directory);
        printf("---------------------FILES-----------------------\n");
    }
    else {
        perror("Directory empty or doesn't exist !\n");
        pthread_exit(NULL);
    }
    
    /* Asking for valid filename */
    while(file == NULL){
        /* Asking for a file */
        printf("Type a filename (or /abort) : ");
        fgets(filename, MAX_BUFFER_LENGTH, stdin);

        /* Client want to continue ? */
        *strchr(filename, '\n') = '\0';
        if(strcmp(filename,"/abort")==0){
            pthread_exit(NULL);
        }

        /* Open the selected file */
        file = fopen(filename, "rb");
    }

    /* Init the protocol for sending file */
    sd = send(*arg,&file_protocol,strlen(file_protocol),0);
    if(sd == 0){
        /* Connexion lost */
        pthread_exit(NULL);
    }

    /* MEGA IMPORTANT ! */
    nanosleep(&tim , &tim2);

    /* Then send the filename */
    sd = send(*arg,&filename,sizeof(filename),0);
    if(sd == 0){
        /* Connexion lost */
        pthread_exit(NULL);
    }

    /* Get file length */
	fseek(file, 0, SEEK_END);
    int size = ftell(file);
    char* size_c = (char*)&size;
	printf("sending %d bytes...\n",size);
	fseek(file, 0, SEEK_SET);

    /* Then send the size */
    sd = send(*arg,size_c,sizeof(size_c),0);
    if(sd == 0){
        /* Connexion lost */
        pthread_exit(NULL);
    }

    /* Then send the content */
    while (size > sizeof(buffer)){
        /* Sending packets */
        fread(buffer, sizeof(buffer), 1, file);
        nanosleep(&tim , &tim2);
        sd = send(*arg,&buffer,sizeof(buffer),0);
        if(sd == 0){
            /* Connexion lost */
            pthread_exit(NULL);
        }
        size-= sizeof(buffer);
    }

    fread(buffer, size, 1, file);
    nanosleep(&tim , &tim2);
    sd = send(*arg,&buffer,size,0);
    if(sd == 0){
        /* Connexion lost */
        pthread_exit(NULL);
    }

    /* MEGA IMPORTANT ! */
    nanosleep(&tim , &tim2);

    /* finally, end the the protocol */
    while(sd = send(*arg,&file_protocol,strlen(file_protocol),0) < strlen(file_protocol)){
        if(sd == 0){
            /* Connexion lost */
            pthread_exit(NULL);
        }
    }

    /* Close the file */
    fclose(file);
    printf("\bFile sent !\n");
    pthread_exit(NULL);
}

void *sendMsg(void* dS){
    /* Get server's socket and declare some variables */
    int* arg = dS;
    char buffer[MAX_BUFFER_LENGTH];
    int sd;

    while(1){
        /* Clean the buffer */
        memset(buffer, 0, strlen(buffer));
        
        /* Get the message to send */
        fgets(buffer, MAX_BUFFER_LENGTH, stdin);

        /* Check if it's the end of communication */
        char *chfin = strchr(buffer, '\n');
        *chfin = '\0';
        if(strcmp(buffer,"fin") == 0){
            //Disconnect client.
            sd = send(*arg,&buffer,strlen(buffer),0);
            if (sd < 0){
                perror("! Error with sending 'fin' !");
            }
            //Send num channel
            sd = send(*arg,&buffer,strlen(buffer),0);
            if (sd < 0){
                perror("! Error with sending 'fin' !");
            }
            printf("End of the communication ...\n");
            break;
        }

        /* Check if it is a file */
        else if(strcmp(buffer,"/file") == 0){

            /* Create a thread */
            pthread_t sdF;
            pthread_create(&sdF,0,sendFile,arg);
            pthread_join(sdF,0);
        }

        /* It's a simple text message */
        else{
            /* Send the message */
            while(sd = send(*arg,&buffer,strlen(buffer),0) <= strlen(buffer)-1){
                if(sd == 0){
                    /* Connexion lost */
                    break;
                }
            }
        }
    }
    pthread_exit(NULL);
}

void *recvFile(void* dS){
    /* Get server's socket */
    int* arg = dS;
    char buffer[MAX_BUFFER_LENGTH];
    int rc;
    char filename[MAX_BUFFER_LENGTH];
    char file_protocol[FILE_PROTOCOL_LENGTH] = FILE_PROTOCOL;
    FILE* file;
    int size;
    char* size_c = (char*)&size;

    /* Client recieve a file */
    printf("Recieving a file...\n");
    memset(buffer, 0, sizeof(buffer));
    memset(filename, 0, sizeof(filename));

    /* Get the filename */
    rc = recv(*arg, &filename, sizeof(filename), 0);
    if(rc == 0){
        /* Connexion lost */
        pthread_exit(NULL);
    }

    /* Get the size */
    rc = recv(*arg, size_c, sizeof(size_c), 0);
    if(rc == 0){
        /* Connexion lost */
        pthread_exit(NULL);
    }

    printf("Total : %d bytes\n",size);

    /* Show the filename and create the new file */
    printf("Filename : %s\n", filename);
    file = fopen(filename, "wb");

    /* Write in the new file */
    if(file == NULL){
        perror("Error creating the new file...\n");
        pthread_exit(NULL);
    }

    /* Flushing */
    recv(*arg, &buffer, sizeof(buffer), 0);

    /* Get chunks */
    while(size > sizeof(buffer)){
        recv(*arg, &buffer, sizeof(buffer), 0);
        fwrite(buffer, sizeof(buffer), 1, file);
        size = size - MAX_BUFFER_LENGTH;
    }

    /* Get last chunk */
    recv(*arg, &buffer, size, 0);
    fwrite(buffer, size, 1, file);

    /* Finish file transfert */
    fclose(file);
    printf("\bFile transfered !\n");
    pthread_exit(NULL);
}

void *recvMsg(void* dS){
    /* Get server's socket */
    int* arg = dS;
    char buffer[MAX_BUFFER_LENGTH + JOINER_LENGTH + MAX_NAME_LENGTH];
    int rc;
    char file_protocol[FILE_PROTOCOL_LENGTH] = FILE_PROTOCOL;

    while(1){
        /* Clean the buffer */
        memset(buffer, 0, strlen(buffer));

        /* Recieve the message */
        while(rc = recv(*arg, &buffer, sizeof(buffer), 0) <= 0){
           if(rc == 0){
                /* Connexion lost */
                break;
           }
        }

        /* Client recieve a file */
        if(strcmp(buffer, file_protocol) == 0){

            /* Create thread */
            pthread_t rcF;
            pthread_create(&rcF,0,recvFile,arg);
            pthread_join(rcF,0);
            
		}
        /* Client recieve simple text message */
        else{
            /* Print the message */
            printf("%s\n", buffer);
        }
    }
}

int main(int argc, char *argv[]){

    /* Checking args */
    if(argc != 3){
        printf("! I need to be call like -> :program SERVER_IP PORT !\n");
        exit(1);
    }

    /* Ask for username */
    char username[MAX_NAME_LENGTH];
    printf("Choose your username : ");
    fgets(username, MAX_NAME_LENGTH, stdin);

    //* Define target (ip:port) with calling program parameters */
    struct addrinfo hints, *res, *p;
    int status;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // the version of ip addresses isn't specified
    hints.ai_socktype = SOCK_STREAM; //TCP

    /*Identify current address*/
    if ((status = getaddrinfo(argv[1], argv[2], &hints, &res)) != 0) { //recover the version of the given ip
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    exit(EXIT_FAILURE);
    }

    p = res;
    int dS;
      
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

    /*Free memory*/
    freeaddrinfo(res); 

    /* Send username to the server */
    int sd,rc;
    while(sd = send(dS,&username,strlen(username)-1,0) <= strlen(username)-2){
        if(sd == 0){
            /* Connexion lost */
            exit(1);
        }
    }

    char list[MAX_BUFFER_LENGTH];
    /* Receive the channel list */
    rc = recv(dS, &list, sizeof(list), 0);
    if(rc<0){
        printf("! Error receiving the channel list !\n");
    }
    puts(list);

    sleep(1);

    /* Choose the channel the client want to join */
    char channelChoice[MAX_NAME_LENGTH];
    printf("Enter the name of a channel you want to join : ");
    fgets(channelChoice, MAX_NAME_LENGTH, stdin);

    /* Send the choice to the server */
    sd = send(dS,&channelChoice,sizeof(channelChoice),0);
    if(sd < 0){
        printf("! Error sending the chosen channel to server !\n");
    }

    int repChannel;
    /* Receive the response from the server */
    rc = recv(dS, &repChannel, sizeof(repChannel), 0);
    if(rc < 0){
        printf("! Error receiving the answer of the server for the channel !\n");
    }

    /* The server find an error to connect to the channel */
    while(repChannel < 0){
        printf("Error ! The channel doesn't exist or it is full !\n");

        /* Choose an other channel */
        printf("Enter the name of a channel you want to join : ");
        memset(channelChoice, 0, MAX_NAME_LENGTH);
        fgets(channelChoice, MAX_NAME_LENGTH, stdin);

        /* Send the new choice to the server */
        sd = send(dS,&channelChoice,sizeof(channelChoice),0);
        if(sd < 0){
            printf("! Error sending the chosen channel to server !\n");
        }

        /* Receive the response from the server */
        rc = recv(dS, &repChannel, sizeof(repChannel), 0);
        if(rc < 0){
            printf("! Error receiving the answer of the server for the channel !\n");
        }
    }

    if(repChannel >= 0){
        printf("You are connected at %s\n",channelChoice);
        printf("You can chat now !\n");
        printf("To leave the app, enter 'fin' in your terminal.\n\n");
    }


    /* Create 2 threads for receive and send messages */
    pthread_t sdM;
    pthread_t rcM;
    pthread_create(&sdM,0,sendMsg,&dS);
    pthread_create(&rcM,0,recvMsg,&dS);

    /* Waiting for the end of communication to finish the program */
    pthread_join(sdM,0);  

    /* Close connexion */
    close(dS);
    return 1;
}