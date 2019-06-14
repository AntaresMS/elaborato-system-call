#include <stdio.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>


int main(){
	int msqid[5];

	srand(time(NULL));

	for(int i = 0; i < 5; i++){
		key_t key = ftok("clientExec.c", rand() % 1000);
		msqid[i] = msgget(key, IPC_CREAT | S_IRUSR | S_IWUSR);
	} 
}