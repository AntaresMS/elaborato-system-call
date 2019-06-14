#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "../inc/client_server_lib.h"
#include "../inc/errExit.h"


int main (int argc, char *argv[]) {

    // ===== OPERAZIONI PRELIMINARI =====

    printf("Hi, I'm Server program!\n");

    // blocca tutti i segnali, a eccezione di SIGTERM
    sigset_t signalSet;

    sigfillset(&signalSet);
    sigdelset(&signalSet, SIGTERM);
    sigprocmask(SIGTERM, &signalSet, NULL);

    // imposto sigHandler per la gestione di SIGTERM
    if(signal(SIGTERM, signalHandler_server) == SIG_ERR){
        errExit("change signal handler failed");
    }


    // crea FIFOSERVER
    printf("Creating FIFOSERVER... \n");
    if (mkfifo("FIFOSERVER", S_IRUSR | S_IWUSR) == -1){
    	errExit("mkfifo failed");
    }

    // creo un segmento di memoria condivisa
    printf("Creating shared memory segment...\n");

    // genero una chiave per il segmento di memoria condivisa
    key_t shmKey = ftok("src/functions.c", 300);
    if(shmKey == -1){
        errExit("fotk failed");
    }

    shm_id = shmget(shmKey, sizeof(struct entry) * N_ENTRY, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    if(shm_id == -1){
        errExit("shmget failed");
    }

    // attacco il segmento al server
    shmaddr_server = (struct entry *)shmat(shm_id, NULL, 0);    // terzo argomento = 0 : imposto permessi di lettura e scrittura
    if(shmaddr_server == (struct entry *)-1){
        errExit("shmat failed");
    }

    // creo semaforo di mutua esclusione, inizializzato a 1
    printf("Creating semaphore...\n");

    // genero una chiave per il semaforo
    key_t semKey = ftok("src/server.c", 100);
    if(semKey == -1){
        errExit("ftok failed");
    }

    mutex = semget(semKey, 1, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
    union semun arg;
    arg.val = 1;
    if(semctl(mutex, 0, SETVAL, arg) == -1){
        errExit("(mutex) semctl failed");
    }

    // ======================


    // genero il processo figlio KeyManager 
    key_manager = fork();


    if(key_manager == -1){
        errExit("fork failed");
    // ===== KEYMANAGER =====
    } else if(key_manager == 0){     
        printf("\nKeyManager is running!\n");

        // imposto sigHandler per la gestione di SIGTERM
        if(signal(SIGTERM, signalHandler_keyman) == SIG_ERR){
            errExit("change signal handler failed");
        }

        // attacco il segmento di memoria condivisa
        struct entry *shmaddr_keyman = (struct entry *)shmat(shm_id, NULL, 0);    // terzo argomento = 0 : imposto permessi di lettura e scrittura
        if(shmaddr_keyman == (struct entry *)-1){
            errExit("shmat failed");
        }

        while(1){
            sleep(30);
            
            // esamino tutte le entry del segmento di memoria condivisa: elimino quelle con timestamp più vecchio di 5 minuti            

            // ------------------- ZONA PROTETTA DAL MUTEX ------------------- //

            semOp(mutex, 0, -1);      

            struct entry *current = shmaddr_keyman;
            int counter = 0;

            while(current != shmaddr_keyman + N_ENTRY){
                time_t min_timestamp = generate_timestamp() - FIVE_MIN;
                if(current->timestamp < min_timestamp && current->timestamp != 0){   
                    remove_entry(current);
                    counter++;
                }
                ++current;
            }

            printMemory(shmaddr_keyman);

            semOp(mutex, 0, 1); 

            // --------------------------------------------------------------- //

            printf("[key manager] check done! %i elements deleted\n", counter);
        }
    // ===== SERVER =====
    } else {   
        while(1){

            // apre FIFOSERVER in lettura
            printf("Reading Request from Client...\n");
            int server_fd = open("FIFOSERVER", O_RDONLY);
            if(server_fd == -1){
            	errExit("FIFOSERVER open failed:");	
            }

            // legge la richiesta
            struct Request buffer_rd[1];
            if(read(server_fd, buffer_rd, sizeof(buffer_rd)) == -1){
            	errExit("read from FIFOSERVER failed");
            }

            // genero la chiave 
            time_t timestamp = generate_timestamp();
            char * service = buffer_rd[0].service;
            int key = generate_key(timestamp, service);

            // ------------------- ZONA PROTETTA DAL MUTEX ------------------- //
            semOp(mutex, 0, -1);
            
            // memorizzo nel segmento di memoria utente, chiave e timestamp
            write_shm(shmaddr_server, buffer_rd[0].user, key, timestamp);
            printMemory(shmaddr_server);

            // rilascio il mutex
            semOp(mutex, 0, 1);      

            // --------------------------------------------------------------- //


            // ===== INVIO CHIAVE AL CLIENT =====

            // scorro quindi tutti gli elementi della directory finché non trovo la FIFO che contiene nel nome il pid del client
            char pidString[5];
            char fifoName[15];
            sprintf(pidString, "%i", buffer_rd[0].pid);

            DIR *dp = opendir(".");

            getFifoName(dp, pidString, fifoName);
            // ora fifoName contiene il nome della fifo 

            // apre FIFOCLIENT in scrittura
            printf("Sending the Response to Client...\n");
            int client_fd = open(fifoName, O_WRONLY);
            if(client_fd == -1){
            	errExit("FIFOCLIENT open failed");
            }

            // invia la chiave al client
            struct Response buffer_wr[1];
            buffer_wr[0].key = key;

            if(write(client_fd, buffer_wr, sizeof(buffer_wr)) == -1){
            	errExit("write on FIFOCLIENT failed");
            }

            // chiude FIFOCLIENT
            close(client_fd);
        }
    }

    return 0;
}
