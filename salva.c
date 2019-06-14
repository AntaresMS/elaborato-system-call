#include <stdlib.h>
#include <stdio.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "../inc/errExit.h"

#define MAX_SIZE 100

// salva su file tutti gli argomenti ricevuti da linea di comando
// argv[]: nome eseguibile | nome del file su cui salvare le informazioni | informazioni da salvare

int main (int argc, char *argv[]) {
	char fileName[MAX_SIZE];
	sprintf(fileName, "%s.txt", argv[1]);    // nome del file: concatenazione di fileName e l'estensione .txt

	// controllo parametri
	if(argc < 3){
    	errExit("wrong input arguments");
    }

    printf("Hi, I'm Salva program!\n");

    // crea un nuovo file
    int fd = open(fileName, O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR);

    if(fd == -1){
    	errExit("open failed");
    }

    // scrive sul file le informazioni passate come argomenti
    int n = 2;
    char buffer[MAX_SIZE];

    strcpy(buffer, argv[n]);
    ssize_t writeBuffer = write(fd, buffer, sizeof(char) * strlen(argv[n]));

    if(writeBuffer != sizeof(char) * strlen(argv[n])){
    	errExit("write failed");
    }    

    printf("Your informations have been correctly saved\n");

    return 0;
}
