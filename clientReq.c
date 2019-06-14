#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "../inc/functions.h"
#include "../inc/errExit.h"

int main (int argc, char *argv[]) {
	struct Request req;


    // ===== INTERFACCIA UTENTE ===== //

    printf("Hi, I'm ClientReq program!\n\n");

    printf("Please provide username and service:\n");

    printf("user: ");
    scanf("%s", req.user);
    printf("service [invia/salva/stampa]: ");
    scanf("%s", req.service);

    req.pid = getpid();

    // controllo la validità di service
    if(strcmp(req.service, "invia") != 0 && strcmp(req.service, "salva") != 0 && strcmp(req.service, "stampa") != 0){
        printf("Invalid service name, please try again\n");
        exit(1);
    }

    printf("\n");



    // ===== INVIO LA RICHIESTA AL SERVER ===== //

    // creo il nome per la FIFO nel formato: <pid>FIFOCLIENT
    char fifoName[15];

    sprintf(fifoName, "%i", req.pid);
    strcat(fifoName, "FIFOCLIENT");

    // crea FIFOCLIENT
    printf("Creating FIFOCLIENT...\n");
    int fifo_client = mkfifo(fifoName, S_IRUSR | S_IWUSR);	// le flag sono di lettura (per client) e scrittura (per server)

    if (fifo_client == -1){
    	errExit("mkfifo failed");
    }

    // apre FIFOSERVER in scrittura
    printf("Sending the Request to Server...\n");
    int server_fd = open("FIFOSERVER", O_WRONLY); // qui il processo si blocca finchè server non apre FIFOSERVER in lettura!
    if(server_fd == -1){		
    	errExit("FIFOSERVER open failed");
    }	


    // invio la richiesta su FIFOSERVER 
    struct Request buffer_wr[] = {req};	// ogni istanza di ClientReq può inviare una sola Request alla volta
    if(write(server_fd, buffer_wr, sizeof(buffer_wr)) == -1){
    	errExit("write on FIFOSERVER failed");
   	}

   	// chiude FIFOSERVER
   	close(server_fd);




    // ===== LETTURA DELLA RISPOSTA ===== //

    // apro FIFOCLIENT in lettura
    printf("Reading the Response from Server...\n");
    int client_fd = open(fifoName, O_RDONLY);	// qui il processo si blocca finché server non apre FIFOCLIENT per inviare la chiave!
    if(client_fd == -1){
    	errExit("FIFOCLIENT open failed");
    }

    struct Response buffer_rd[1];
    if(read(client_fd, buffer_rd, sizeof(buffer_rd)) == -1){
    	errExit("read from FIFOCLIENT failed");
    }

    // memorizzo la chiave letta da FIFOCLIENT
    int key = buffer_rd[0].key;

    printf("\n\nUsername: %s\n", req.user);
    printf("Service: %s\n", req.service);
    printf("Key released from Server: %i\n\n", key);

    // rimuove FIFOCLIENT
    printf("Removing FIFOCLIENT from file system\n");
    if(unlink(fifoName) == -1){
    	errExit("unlink failed");
    }

    printf("Success!\n");


    return 0;
}
