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
	struct Request req;
	struct Response resp;

    printf("Hi, I'm Server program!\n");

    // crea FIFOSERVER: server la usa per leggere i dati, client per inviarli
    printf("Creating FIFOSERVER... \n");
    if (mkfifo("FIFOSERVER", S_IRUSR | S_IWUSR) == -1){
    	errExit("mkfifo failed");
    }

    // creo un segmento di memoria condivisa
    // ...

    // apre FIFOSERVER in lettura
    printf("Reading Request from Client...\n");
    int server_fd = open("FIFOSERVER", O_RDONLY);	// sblocca il processo clientReq
    if(server_fd == -1){
    	errExit("FIFOSERVER open failed:");	
    }

    // legge la richiesta
    struct Request buffer_rd[1];
    if(read(server_fd, buffer_rd, sizeof(buffer_rd)) == -1){
    	errExit("read from FIFOSERVER failed");
    }

    // ottiene chiave - TEMPORANEO!!
    // ------------------------
    resp.key = 1234;
    // ------------------------

    // apre FIFOCLIENT in scrittura
    printf("Sending the Response to Client...\n");
    int client_fd = open("FIFOCLIENT", O_WRONLY);
    if(client_fd == -1){
    	errExit("FIFOCLIENT open failed:");
    }

    // invia la chiave al client
    struct Response buffer_wr[] = {resp};
    if(write(client_fd, buffer_wr, sizeof(buffer_wr)) == -1){
    	errExit("write on FIFOCLIENT failed");
    }

    // chiude FIFOCLIENT
    close(client_fd);

    // rimuove FIFOSERVER
    printf("Removing FIFOSERVER from file system\n");
    if(unlink("FIFOSERVER") == -1){
    	errExit("unlink failed");
    }

    printf("Success!\n");

    return 0;
}
