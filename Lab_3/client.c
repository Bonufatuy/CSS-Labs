#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

#define SIZE_BUF 64

int main(int argc, char* argv[]){
	
	char msg[SIZE_BUF]; 
	int fd = atoi(argv[1]);
	
	int sem = semget(atoi(argv[2]), 0, 0666 | IPC_CREAT);	
	struct sembuf wait = {0, 0, SEM_UNDO};
	struct sembuf lock = {0, 1, SEM_UNDO};
	struct sembuf unlock = {1, -1, SEM_UNDO};

	semop(sem, &lock, 1);
	while(1){
		semop(sem, &wait, 1);
		semop(sem, &lock, 1);
		read(fd, msg, SIZE_BUF);
		
		printf("taken message: %s\n", msg);
		semop(sem, &unlock, 1);
	}
	
	return 0;
}
