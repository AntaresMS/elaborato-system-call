#include <stdlib.h>
#include <stdio.h>

#include "../inc/errExit.h"

// stampa su terminale tutti gli argomenti ricevuti da linea di comando
// argv[]: nome eseguibile | argomenti da stampare

int main (int argc, char *argv[]) {
	int n = 1;

	if(argc < 2){
    	errExit("wrong input arguments");
    }

    printf("Hi, I'm Stampa program!\n");

    while(argv[n] != NULL){
    	printf("\n\t%s ", argv[n]);
    	n++;
    }

    printf("\n\n");

    return 0;
}
