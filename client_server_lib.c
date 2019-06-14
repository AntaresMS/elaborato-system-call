#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/wait.h>

#include "../inc/client_server_lib.h"
#include "../inc/errExit.h"

// FUNZIONI

void semOp (int semid, unsigned short sem_num, short sem_op) {
	// sop contains only 1 element, because we are specifying the operation on just one semaphore!
    struct sembuf sop[1] = { {sem_num, sem_op, 0} };

    if (semop(semid, sop, 1) == -1){
    	errExit("semop failed");
    }
}


void getFifoName(DIR *dp, char pidString[], char buffer[]){

    if (dp == NULL){
        exit(1);  
    } 

    errno = 0;
    struct dirent *dentry;

    // scorro tutte le entry della directory, fino a che non trovo un file il cui nome contiene il pid del processo client
    while ( (dentry = readdir(dp)) != NULL ) {
        if( strstr(dentry->d_name, pidString) != NULL ){
            strcpy(buffer, dentry->d_name);
            break;
        }
    }

    if (errno != 0){
        printf("Error while reading dir.\n");
    }

    closedir(dp);
}


// genera un timestamp per la coppia chiave-utente generata
int generate_timestamp(){
    time_t rawtime;     
    struct tm *info;
    time(&rawtime);
    info = localtime(&rawtime);   

    return (info->tm_hour * 3600) + (info->tm_min * 60) + info->tm_sec ;	// TIMESTAMP: secondi trascorsi dall'inizio della giornata
}


// genera la chiave che permette all'utente di accedere al servizio richiesto
int generate_key(time_t timestamp, char *service){
    // chiave = numero del servizio + numero random tra 1 e 100 * timestamp
    char serviceCode[50];

    if(strcmp(service, "invia") == 0) {
        strcpy(serviceCode, "1");
    } else if(strcmp(service, "salva") == 0) {
        strcpy(serviceCode, "2");
    } else {
        strcpy(serviceCode, "3");
    }

    // genero un numero casuale tra 0 e 999
    srand(time(NULL));
    int randNum = rand() % 1000;

    int result = timestamp + randNum;
    
    // converto result in stringa 
    char buffer[50];
    sprintf(buffer, "%i", result);

    char *string = strcat(serviceCode, buffer);

    return atoi(string); 
}


// memorizza chiave-utente-timestamp nel segmento di memoria condivisa
// @param shm_address    indirizzo del segmento di memoria in cui andrà salvata la entry
// @param username       username della entry
// @param key            chiave della entry
// @param timestamp      timestamp della entry
void write_shm(struct entry *shm_address, char *username, int key, time_t timestamp){
	struct entry * entry = shm_address;
    int i;

	// cerco una entry libera
    // primo caso: chiave già utilizzata, quindi sovrascrivibile
    // secondo caso: entry mai usata
    for(i = 0; i < N_ENTRY; i++){
        if((strcmp(entry->user, "") != 0 && entry->dirtyBit == 1) || strcmp(entry->user, "") == 0){
            break;
        }
        ++entry;
    }

    if(i == N_ENTRY-1){
        printf("Error: full memory\n");
    }

    strcpy(entry->user, username);
	entry->key = key;
	entry->timestamp = timestamp;
    entry->dirtyBit = 0;
}


// signal handler del server
void signalHandler_server(int signal){
    if(signal == SIGTERM){
        printf("\n[server] SIGTERM signal captured, I will terminate now\n");

        // uccisione di KeyManager
        printf("Terminating KeyManager...\n");
        if(kill(key_manager, SIGTERM) == -1){
            errExit("kill failed");
        }

        // rimozione FIFOSERVER
        printf("Removing FIFOSERVER from file system...\n");
        if(unlink("FIFOSERVER") == -1){
            errExit("unlink failed");
        }

        // distacco del server dal segmento di memoria
        printf("Removing shared memory segment...\n");
        if(shmdt(shmaddr_server) == -1){
            errExit("shmadt failed");
        }

        // rimozione del segmento di memoria condivisa
        if(shmctl(shm_id, IPC_RMID, NULL) == -1){
            errExit("shmctl failed");
        }

        // attendo che keymanager termini per evitare che diventi un processo zombie nel caso server termini prima
        if(waitpid(key_manager, NULL, 0) == -1){
            errExit("wait failure");
        }

        exit(0);
    }
}


// signal handler del key manager
void signalHandler_keyman(int signal){
    if(signal == SIGTERM){
        printf("\n[keyManager] SIGTERM signal captured, I will terminate now\n");
        exit(0);
    }
}


// rimuove una entry dal segmento di memoria condivisa
// @param address    indirizzo di memoria della entry da rimuovere
void remove_entry(struct entry *address){
    address->dirtyBit = 1;
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

