#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "errExit.h"

#define MAX_SZ 30

struct Request{
	char username[MAX_SZ];
	char service[MAX_SZ];
};

struct Response{
	int key;
};

int main (int argc, char *argv[]) {
	int choice;
	struct Request req;
	struct Response resp;

    printf("Hi, I'm ClientReq program!\n\n");

    printf("Please choose between these available services:\n");
    printf("[1] invia\t[2] salva\t[3] stampa\n");

    do{
    	scanf("%i", &choice);
    	if(choice < 1 || choice > 3){
    		printf("Invalid option, try again\n");
    	}
    }while(choice < 1 || choice > 3);

    printf("Please provide these data:\n");
    
    printf("username: ");
    scanf("%s", req.username);

    printf("service: ");
    scanf("%s", req.service);

    printf("\n");

    // inoltra i dati al server, ottiene chiave, stampa chiave sul terminale

    // crea FIFOCLIENT: server la usa per inviare dati, client per leggerli
    printf("Creating FIFOCLIENT...\n");
    int fifo_client = mkfifo("FIFOCLIENT", S_IRUSR | S_IWUSR);	// le flag sono di lettura (per client) e scrittura (per server)

    if (fifo_client == -1){
    	errExit("mkfifo failed");
    }

    // apre FIFOSERVER in scrittura
    printf("Sending the Request to Server...\n");
    int server_fd = open("FIFOSERVER", O_WRONLY); // qui il processo si blocca finchè server non apre FIFOSERVER in lettura!
    if(server_fd == -1){		
    	errExit("FIFOSERVER open failed");
    }	


    // scrivo una struct Request su FIFOSERVER 
    struct Request buffer_wr[] = {req};	// ogni istanza di ClientReq può inviare una sola Request alla volta
    if(write(server_fd, buffer_wr, sizeof(buffer_wr)) == -1){
    	errExit("write on FIFOSERVER failed");
   	}

   	// chiude FIFOSERVER
   	close(server_fd);

    // apro FIFOCLIENT in lettura
    printf("Reading the Response from Server...\n");
    int client_fd = open("FIFOCLIENT", O_RDONLY);	// qui il processo si blocca finché server non apre FIFOCLIENT per inviare la chiave!
    if(client_fd == -1){
    	errExit("FIFOCLIENT open failed");
    }

    struct Response buffer_rd[1];
    if(read(client_fd, buffer_rd, sizeof(buffer_rd)) == -1){
    	errExit("read from FIFOCLIENT failed");
    }

    // memorizzo la chiave letta da FIFOCLIENT
    int key = buffer_rd[0].key;

    printf("\nKey released from Server: ");
    printf("%i\n\n", key);

    // rimuove FIFOCLIENT
    printf("Removing FIFOCLIENT from file system\n");
    if(unlink("FIFOCLIENT") == -1){
    	errExit("unlink failed");
    }

    printf("Success!\n");


    return 0;
}
