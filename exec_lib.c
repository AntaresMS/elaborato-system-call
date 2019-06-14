#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>

#include "../inc/exec_lib.h"
#include "../inc/errExit.h"

// FUNZIONI

void semOp (int semid, unsigned short sem_num, short sem_op) {
    // sop contains only 1 element, because we are specifying the operation on just one semaphore!
    struct sembuf sop[1] = { {sem_num, sem_op, 0} };

    if (semop(semid, sop, 1) == -1){
        errExit("semop failed");
    }
}


// converte gli argomenti da stampare/salvare/inviare in un'unica stringa
// @param array[]  array dei puntatori agli argomenti
// @param n        indice del puntatore al primo argomento valido
char *from_args_to_string(char *array[], int n){
    int length = msg_size(array, n);
    char *result = (char *) malloc(length * sizeof(char) + 1);

    while(array[n] != NULL){
        result = strcat(result, array[n]);
        result = strcat(result, " ");   // inserisco gli spazi tra le stringhe
        n++;
    }

    return result;
}


// calcola la lunghezza del messaggio da inviare
// @param array[]  array dei puntatori agli argomenti
// @param n        indice del puntatore al primo argomento valido
int msg_size(char *array[], int n){
    int size = 0; 

    // somma la lunghezza di tutte le stringhe passate in argomento
    while(array[n] != NULL){
        size += strlen(array[n]);
        n++;
    }

    return size;
}


// controlla che la coppia (user, key) sia presente nella memoria condivisa
// @param *address indirizzo di memoria del segmento di memoria condivisa
// @param *user    nome utente
// @param key      chiave
struct entry * entry_address(struct entry *address, char *user, int key){
    struct entry *current = address;

    while(current != address + N_ENTRY){
        if(strcmp(current->user, user) == 0 && current->key == key){
            return current;
        }
        ++current;
    }

    return NULL;

}


// verifica che la chiave sia ancora valida
// @param *address  indirizzo della entry di cui bisogna controllare il dirty bit
bool dirtyBit_isZero(struct entry *address){
    return address->dirtyBit == 0;
}


void printMemory(struct entry *shm_address){
    struct entry * current = shm_address;

    while(current != shm_address + N_ENTRY){
        printf("user: %s\t\t", current->user);
        printf("key: %i\t\t", current->key);
        printf("timestamp: %li\t\t", current->timestamp);
        printf("dirtyBit: %i\n", current->dirtyBit);
        ++current;
    }
}
