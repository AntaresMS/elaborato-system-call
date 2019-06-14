#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "../inc/functions.h"
#include "../inc/errExit.h"


int main (int argc, char *argv[]) {

    // utente deve fornire: username | chiave rilasciata dal server | lista argomenti per utilizzare il servizio
    if(argc < 4){
        errExit("wrong input arguments");
    }

	int choice;
    char *user = argv[1];
    int server_key = atoi(argv[2]);

    printf("Hi, I'm ClientExec program!\n");

    // ottengo la chiave per il segmento di memoria condivisa
    key_t shmKey = ftok("../clientReq-server/src/functions.c", 300);

    // ottengo il suo identificatore
    shm_id = shmget(shmKey, sizeof(struct entry) * N_ENTRY, S_IRUSR | S_IWUSR);

    if(shm_id == -1){
        errExit("shmget failed");
    }

    // attacco clientExec al segmento di memoria condivisa
    struct entry *shmaddr_clientexec = (struct entry *)shmat(shm_id, NULL, 0);
    if(shmaddr_clientexec == (struct entry *)-1){
        errExit("shmat failed");
    }

    // ottengo la chiave per il semaforo
    key_t semKey = ftok("../clientReq-server/src/server.c", 100);

    // ottengo il semaforo
    mutex = semget(semKey, 1, S_IRUSR | S_IWUSR);
    if(mutex == -1){
        errExit("(mutex) semget failed");
    }

    // verifica validità della chiave

    // ------------------- ZONA PROTETTA DAL MUTEX ------------------- //

    semOp(mutex, 0, -1);

    // CONDIZIONI DI VALIDITÀ DI UNA CHIAVE: 
    //     la coppia (utente, chiave) è presente nella memoria condivisa
    //     non è ancora stata utilizzata --> dirtyBit == 0
    struct entry * myEntry = entry_address(shmaddr_clientexec, user, server_key);

    if(myEntry == NULL){
        printf("Error: [user, key] are not present in the system\nPlease require another key\n");
        semOp(mutex, 0, 1);
        exit(1);
    } else if (dirtyBit_isZero(myEntry) == false){
        printf("Error: this key has already been used\nPlease require another key\n");
        semOp(mutex, 0, 1);
        exit(1);
    } else {
        myEntry->dirtyBit = 1;      // siccome la chiave è valida, la uso e pongo il suo dirtyBit a 1 in modo che non sia più possibile usarla in futuro
    }

    semOp(mutex, 0, 1);

    // ---------------------------------------------------------------- //

    printf("What kind of service do you want to use?\n");
    printf("[1] invia\t[2] salva\t[3] stampa\n");
    while(choice < 1 || choice > 3){
  		scanf("%i", &choice);
  		if(choice < 1 || choice > 3){
  			printf("Invalid option, try again\n");
  		}
    }

    // converto gli altri argomenti da passare al servizio in un'unica stringa
    char *arguments = from_args_to_string(argv, 3);

    // creo un figlio, e faccio in modo che esegua il codice di invia.c, salva.c, oppure stampa.c
    
    switch(choice){
    	case 1:{
            int msqKey;
            printf("Please provide the message queue key:\n");
            scanf("%i", &msqKey);

            // converto la chiave fornita in una stringa, in modo da poterla passare come argomento alla exec
            char key_str[30];
            sprintf(key_str, "%i", msqKey);

    		if(execlp("./invia", "./invia", key_str, arguments, (char*)NULL) == -1){
                errExit("execlp failure");
            }
    		// padre
    		break;
    	}
    	case 2:{
    		char name[FILE_LENGTH];
    		printf("Please provide the name of the file where you want to save your informations [max 50 characters]:\n");
    		scanf("%s", name);

    		if(execlp("./salva", "./salva", name, arguments, (char*)NULL) == -1){
                errExit("execlp failure");
    		}
    		// padre
    		break;
    	}
	   	default:{
	   	   if(execlp("./stampa", "./stampa", arguments, (char*)NULL) == -1){
                errExit("execlp failure");
    		}
    	}
    }

    return 0;
}
