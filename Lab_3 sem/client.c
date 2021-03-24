#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>

union semun{
	int val;
	struct semid* buff;
	unsigned short *array;
	struct seminfo* buf;
};

int main(int argc, char* argv[]){
	
	char msg[64]; 
	int fd = atoi(argv[1]);
	int semid = semget(atoi(argv[2]), 0, 0666 | IPC_CREAT);
	
	struct sembuf lock = {0, -2, 0};
	struct sembuf rel_res = {0, 1, 0};

	while(1){
		if(semctl(semid, 0, GETVAL, 0) == 2){
			semop(semid, &lock, 1);
			read(fd, msg, 64);
			printf("taken message: %s\n", msg);
			semop(semid, &rel_res, 1);
		}
	}
	
	exit(0);
}
