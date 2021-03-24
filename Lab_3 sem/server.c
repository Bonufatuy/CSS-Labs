#include <stdio.h>
#include <sys/sem.h>
#include <stdlib.h>

union semun{
	int val;
	struct semid* buff;
	unsigned short *array;
	struct seminfo* buf;
};

int _strlen(char s[]) {
	int i;
	i = 0;
	while (s[i] != '\0')
		++i;
	return i;
}

void _reverse(char s[]){
	int i, j;
	char h;
	for (i = 0, j = _strlen(s)-1; i < j; i++, j--) {
 		h = s[i];
		s[i] = s[j];
		s[j] = h;
	}
}

void _itoa (int n, char s[]) {
	int i, sign;
	if ((sign = n) < 0){
		n = -n;
	}
	i = 0;
 	do { 
		s[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);
	if (sign < 0)
	s[i++] = '-';
	s[i] = '\0';
	_reverse(s);
}

int main(){

	int semid;
	int key;
	union semun arg;
    
	struct sembuf lock = {0, -1, 0};
	struct sembuf rel_res = {0, 2, 0};
	
	if((key = ftok(".", 'S')) < 0){
		printf("Невозможно получить ключ\n");
		exit(1);
	}
	
	semid = semget(key, 1, 0666 | IPC_CREAT);
	arg.val = 1;
	
	if(semctl(semid, 0, SETVAL, arg) == -1)
		printf("Error semctl!");
    
	pid_t pid, mainPid = getpid();
	int fd[2];
	char buff[32], str[20], key_buff[32];
	
	if(pipe(fd) < 0){
		printf("Pipe error\n");
		exit(0);
	}
	if((pid = fork()) < 0){
		printf("fork error\n");
		exit(0);
	}else if(pid == 0){
		close(fd[1]);
		_itoa(fd[0], buff);
		_itoa(key, key_buff);
		execl("client", " ", buff, key_buff, NULL);
	}else{
		close(fd[0]);
		while(1){		
			if(semctl(semid, 0, GETVAL, 0) == 1){
				semop(semid, &lock, 1);
				printf("Введите слово: ");
				scanf("%21s", &str);
				while(getchar() != '\n');
				str[21] = '\0';
				write(fd[1], str, 10);	
				semop(semid, &rel_res, 1);
			}					 
        }	
	}

    exit(0);
}
