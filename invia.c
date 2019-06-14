#include <stdlib.h>
#include <stdio.h>

#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "../inc/errExit.h"

// creo la struct contente il messaggio da inviare
struct mymsg {
   	long mtype;
   	char *mtext;
};


// deposita gli argomenti in una coda di messaggi già esistente nel sistema, identificata da una precisa chiave
// argv[]: nome eseguibile | chiave della coda di messaggi | argomenti da inviare

int main (int argc, char *argv[]) {
	key_t key = atoi(argv[1]);
	char *message = argv[2];

	// controllo parametri
	if(argc < 3){
    	errExit("wrong input arguments");
    }

    printf("Hi, I'm Invia program!\n");

    // ottengo l'identificatore della coda di messaggi
    int msq_id = msgget(key, S_IWUSR);

    if(msq_id == -1){
        errExit("msgget failed");
    }

	struct mymsg msg_send;
    msg_send.mtype = 1;
    msg_send.mtext = message;

    printf("sending your message...\n");
    if(msgsnd(msq_id, &msg_send, sizeof( msg_send.mtext), 0) == -1){
        errExit("msgsnd failure");
    }

    printf("Message sent successfully through the message queue\n");

    return 0;
}
