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
#define MAX_BUFFER_LENGTH 256
#define MAX_LIST_LENGTH 1200
#define JOINER_LENGTH 4
#define FILE_PROTOCOL_LENGTH 10
#define FILE_PROTOCOL "-FILE-"

/* Frequency for sending file chunks => 200Hz */
#define FREQUENCY 50000000L

int continueMenu=1;

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
            continueMenu=0;
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

int num_command_channel(char str[]){
    char *chfin = strchr(str, '\n');
    *chfin = '\0';

    if(strcmp(str,"/enter") == 0){
        return 1;
    } else if(strcmp(str,"/add") == 0) {
        return 2;
    } else if (strcmp(str,"/delete") == 0) {
        return 3;
    } else if (strcmp(str,"/update") == 0) {
       return 4;
    } else {
        return -1;
    }
}

void enter_channel(int dS) {
    int rc,sd;

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

    continueMenu=0;
    /* Create 2 threads for receive and send messages */
    pthread_t sdM;
    pthread_t rcM;
    pthread_create(&sdM,0,sendMsg,&dS);
    pthread_create(&rcM,0,recvMsg,&dS);

    /* Waiting for the end of communication to finish the program */
    pthread_join(sdM,0);
}

void add_channel(int dS){
    int sd;

    /* Choose the new name */
    printf("You choose to add a new channel !\nChoose a name (or /abort) :\n");
    char channelName[MAX_NAME_LENGTH];
    fgets(channelName, MAX_NAME_LENGTH, stdin);

    char *chfin = strchr(channelName, '\n');
    *chfin = '\0';

    /* Send the name to the server */
    sd = send(dS,&channelName,sizeof(channelName),0);
    if(sd < 0){
        printf("! Error sending the chosen channel to server !\n");
    }

    /* If client don't abort, continue to add */
    if(strcmp(channelName,"/abort") != 0){

        /* Choose the new description */
        printf("Choose a description :\n");
        char channelDescription[MAX_NAME_LENGTH];
        fgets(channelDescription, MAX_NAME_LENGTH, stdin);

        sd = send(dS,&channelDescription,sizeof(channelDescription),0);
        if (sd < 0){
            perror("! Error with sending 'fin' !");
        }

        /* Choose the new max clients */
        printf("Choose a max of clients :\n");
        char maxCliStr[4];
        fgets(maxCliStr, 4, stdin);

        int maxCli = atoi(maxCliStr); /* Convert to int */
        while(maxCli == 0){
            puts("Invalid number of max clients.\n");
            puts("Choose a max of clients :\n");
            fgets(maxCliStr, 4, stdin);
            maxCli = atoi(maxCliStr);
        }

        sd = send(dS,&maxCli,sizeof(maxCli),0);
        if (sd < 0){
            perror("! Error with sending 'fin' !");
        }

        printf("New channel created !\n");
    }
}

void delete_channel(int dS){
    int sd,rc;

    char list[MAX_LIST_LENGTH];
    /* Receive the channel list from the server */
    rc = recv(dS, &list, sizeof(list), 0);
    if(rc < 0){
        printf("! Error receiving the answer of the server for the channel !\n");
    }
    printf("%s\n",list);

    /* Choose the channel name */
    printf("Choose a channel to delete :\n");
    char channelName[MAX_NAME_LENGTH];
    fgets(channelName, MAX_NAME_LENGTH, stdin);

    sd = send(dS,&channelName,sizeof(channelName),0);
    if (sd < 0){
        perror("! Error with sending 'fin' !");
    }

    char resp[MAX_BUFFER_LENGTH];
    /* Receive the response from the server */
    rc = recv(dS, &resp, sizeof(list), 0);
    if(rc < 0){
        printf("! Error receiving the answer of the server for the channel !\n");
    }
    printf("%s\n",resp);
}

void update_attr_channel(char chosenName[], int dS){
    int sd;

    /* Switch case of client command choice */
    /* Change name */
    if(strcmp(chosenName,"/name")==0){
         printf("Choose a new name : \n");
         char channelName[MAX_NAME_LENGTH];
         fgets(channelName, MAX_NAME_LENGTH, stdin);

         sd = send(dS,&channelName,sizeof(channelName),0);
         if (sd < 0){
             perror("! Error with sending new name for updated channel !");
         }

         printf("Channel name is updated!\n");

    }
    /* Change description */
    else if(strcmp(chosenName,"/description")==0){
        printf("Choose a new description : \n");
        char channelDescription[MAX_BUFFER_LENGTH];
        fgets(channelDescription, MAX_BUFFER_LENGTH, stdin);

        sd = send(dS,&channelDescription,sizeof(channelDescription),0);
        if (sd < 0){
         perror("! Error with sending new description for updated channel !");
        }

        printf("Channel description is updated!\n");

    }
    /* Change max clients */
    else if(strcmp(chosenName,"/maxClients")==0){
        printf("Choose a new max clients : \n");
        char channelMaxCliStr[4];
        fgets(channelMaxCliStr, sizeof(channelMaxCliStr), stdin);

        int channelMaxCli = atoi(channelMaxCliStr);
        while(channelMaxCli==0){
            printf("Invalid max clients\n");
            printf("Choose a new max clients : \n");
            memset(channelMaxCliStr,0,4);
            fgets(channelMaxCliStr, sizeof(channelMaxCliStr), stdin);

            channelMaxCli = atoi(channelMaxCliStr);
        }

        sd = send(dS,&channelMaxCli,sizeof(channelMaxCli),0);
        if (sd < 0){
         perror("! Error with sending new max clients for updated channel !");
        }

        printf("Channel max clients is updated!\n");

    }
    /* Error */
    else {
        printf("! Error, invalid command !\n");
    }

}

void update_channel(int dS) {
    int sd,rc;

    /* Channel name choice */
    printf("Choose a channel to update :\n");
    char channelName[MAX_NAME_LENGTH];
    fgets(channelName, MAX_NAME_LENGTH, stdin);

    sd = send(dS,&channelName,sizeof(channelName),0);
    if (sd < 0){
        perror("! Error with sending 'fin' !");
    }

    int canUp;
    /* Receive the response from the server */
    rc = recv(dS, &canUp, sizeof(canUp), 0);
    if(rc < 0){
        printf("! Error receiving the answer of the server for the channel !\n");
    }

    if(canUp < 0){
        printf("The channel doesn't exist !\n");
    } else if (canUp == 0){
        printf("You can't update the channel if clients are connected in !\n");
    } else {
        printf("Choose the attribute(s) to change :\n '/name', '/description', '/maxClients'\n");

        memset(channelName,0,MAX_NAME_LENGTH);
        fgets(channelName, MAX_NAME_LENGTH, stdin);

        char *chfin = strchr(channelName, '\n');
        *chfin = '\0';

        /* Send the command */
        sd = send(dS,&channelName,sizeof(channelName),0);
        if (sd < 0){
            perror("! Error with sending the chosen command to update !");
        }

        update_attr_channel(channelName,dS);
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

    while(continueMenu==1){

        char list[MAX_LIST_LENGTH];
        /* Receive the channel list */
        rc = recv(dS, &list, sizeof(list), 0);
        if(rc<0){
            printf("! Error receiving the channel list !\n");
        }
        printf("%s\n",list);
        memset(list,0,MAX_LIST_LENGTH);

        /* Choose how to manage channel */
        char choice[MAX_NAME_LENGTH];
        sleep(1);
        printf("Choose an alternative : \n '/enter' to join a channel \n '/add' to create a new channel \n '/delete' or '/update' a channel \n\n");
        fgets(choice, MAX_NAME_LENGTH, stdin);

        int numChoice = num_command_channel(choice);
        while (numChoice == -1){
            memset(choice, 0, MAX_NAME_LENGTH);
            printf("This command is invalid. Please choose between : /enter, /add, /delete\n\n");
            fgets(choice, MAX_NAME_LENGTH, stdin);
            numChoice = num_command_channel(choice);
        }

        /* Send the choice to the server */
        sd = send(dS,&numChoice,sizeof(numChoice),0);
        if(sd < 0){
            printf("! Error sending the chosen action to server !\n");
        }

        switch (numChoice){
            case 1 :
                enter_channel(dS);
                break;

            case 2 :
                add_channel(dS);
                break;

            case 3 :
                delete_channel(dS);
                break;

            case 4 :
                update_channel(dS);
                break;

            default :
                printf("! Error, invalid command enter by the client !\n");
                exit(1);
        }
    }

    /* Close connexion */
    close(dS);
    return 1;
}