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

/**
* This version allows a client to send messages and files
* Run the program : gcc -o client client.c -lpthread && ./client TARGET_IP PORT
*/

#define MAX_USERNAME_LENGTH 20
#define MAX_BUFFER_LENGTH 256
#define JOINER_LENGTH 4

void *sendMsg(void* dS){
    /* Get server's socket and declare some variables */
    int* arg = dS;
    char buffer[MAX_BUFFER_LENGTH];
    int sd;
    char filename[MAX_BUFFER_LENGTH];
    char file_protocol[10] = "-FILE-";
    DIR* directory;
	FILE* file;

    while(1){
        /* Clean the buffer */
        memset(buffer, 0, strlen(buffer));
        
        /* Get the message to send */
        fgets(buffer, MAX_BUFFER_LENGTH, stdin);

        /* Check if it's the end of communication */
        char *chfin = strchr(buffer, '\n');
        *chfin = '\0';
        if(strcmp(buffer,"fin") == 0){
            printf("End of the communication ...\n");
            break;
        }

        if(strcmp(buffer,"/file") == 0){

            memset(filename, 0, MAX_BUFFER_LENGTH);

            /* Print files in the current directory */
			directory = opendir (".");
			if (directory != NULL){
				struct dirent * dir_f;
				while ((dir_f = readdir(directory)) != NULL){					
                    printf("%s\n", dir_f->d_name);
				}
				closedir(directory);
			}
            else {
				perror ("Directory empty or doesn't exist !\n");
                break;
			}
			
            /* Asking for a file */
            printf("Choose a file : ");
            fgets(filename, MAX_BUFFER_LENGTH, stdin);

            /* Open the selected file */
            *strchr(filename, '\n') = '\0';
            file = fopen(filename, "r");

            if(file == NULL){
                printf("ERROR\n");
            }

            /* Init the protocol for sending file */
            while(sd = send(*arg,&file_protocol,strlen(file_protocol),0) <= strlen(file_protocol)-1){
                if(sd == 0){
                    /* Connexion lost */
                    break;
                }
            }

            printf("protocol sent\n");

            /* Then send the filename */
            while(sd = send(*arg,&filename,strlen(filename),0) <= strlen(filename)-1){
                if(sd == 0){
                    /* Connexion lost */
                    break;
                }
            }

            /* Then send the content */
            memset(buffer, 0, strlen(buffer));
			while (fgets(buffer, sizeof(buffer), file) != NULL){
                /* Sending packets */
                printf("sending : %s\n",buffer);
                while(sd = send(*arg,&buffer,strlen(buffer),0) <= strlen(buffer)-1){
                    if(sd == 0){
                        /* Connexion lost */
                        break;
                    }
                }
				printf("|");
                memset(buffer, 0, strlen(buffer));
                sleep(1);
			}        

            /* finally, end the the protocol */
            while(sd = send(*arg,&file_protocol,strlen(file_protocol),0) <= strlen(file_protocol)-1){
                if(sd == 0){
                    /* Connexion lost */
                    break;
                }
            }
            printf("Final protocol\n");

            /* Close the file */
			fclose(file);
            printf("\nFile sent\n");
        }

        /* Send the message */
        while(sd = send(*arg,&buffer,strlen(buffer),0) <= strlen(buffer)-1){
           if(sd == 0){
                /* Connexion lost */
                break;
           }
        }
    }
    pthread_exit(NULL);
}

void *recvMsg(void* dS){
    /* Get server's socket */
    int* arg = dS;
    char buffer[MAX_BUFFER_LENGTH + JOINER_LENGTH + MAX_USERNAME_LENGTH];
    int rc;
    char filename[MAX_BUFFER_LENGTH];
    char file_protocol[10] = "-FILE-";
    FILE* file;

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

        if(strcmp(buffer, file_protocol) == 0){
            /* Client recieve a file */
			printf("Ready to recieve a file...\n");
            memset(buffer, 0, strlen(buffer));

            /* Get the filename */
			while(rc = recv(*arg, &filename, sizeof(filename), 0) <= 0){
                if(rc == 0){
                    /* Connexion lost */
                    break;
                }
            }

            /* Show the filename and create the new file */
			printf("Filename : %s\n", filename);
            file = fopen(filename, "w");

            /* Write in the new file */
            if(file != NULL){
                /* Get chunks of the file */
                rc = recv(*arg, &buffer, sizeof(buffer), 0);
                while(strcmp(buffer, file_protocol) != 0){
                    fprintf(file, "%s", buffer);
                    rc = recv(*arg, &buffer, sizeof(buffer), 0);
                    printf("recieve : %s\n",buffer);
                    
                }
                fclose(file);
            }
            else{
				perror("Error creating the new file...\n");
			}
		}
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
    char username[MAX_USERNAME_LENGTH];
    printf("Choose your username : ");
    fgets(username, MAX_USERNAME_LENGTH, stdin);

    /* Define target (ip:port) with calling program parameters */
    struct sockaddr_in aS ;
    aS.sin_family = AF_INET;
    aS.sin_port = htons(atoi(argv[2])) ;
    inet_pton(AF_INET,argv[1],&(aS.sin_addr));
    socklen_t lgA = sizeof(struct sockaddr_in);

    /* Create stream socket with IPv4 domain and IP protocol */
    int dS = socket(PF_INET, SOCK_STREAM, 0);
    if(dS == -1){
		perror("! Issue whith socket creation !\n");
		exit(1);
	}

    /* Open connexion */
    int connexion = connect(dS, (struct sockaddr *) &aS, lgA);
    if(connexion < 0){
        perror("! Can't find the target !\n");
        exit(1);
    }    

    /* Send username to the server */
    int sd;
    while(sd = send(dS,&username,strlen(username)-1,0) <= strlen(username)-2){
        if(sd == 0){
            /* Connexion lost */
            exit(1);
        }
    }
    
    /* Create 2 threads for recieve and send messages */
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