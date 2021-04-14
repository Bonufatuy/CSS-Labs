#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define SIZE_BUF 64

int _strlen(char s[]) {
	int i = 0;
	while (s[i] != '\0') ++i;
	return i;
}

void _reverse(char s[]){
	char h;
	for (int i = 0, j = _strlen(s)-1; i < j; i++, j--) {
 		h = s[i];
		s[i] = s[j];
		s[j] = h;
	}
}

void _itoa (int n, char s[]) {
	int sign, i = 0;
	if ((sign = n) < 0) n = -n;

 	do { 
 		s[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);
	
	if (sign < 0) s[i++] = '-';
	
	s[i] = '\0';
	
	_reverse(s);
}

int main(){
	pid_t pid, mainPid = getpid();
	char buf[SIZE_BUF], str[SIZE_BUF], key_buf[SIZE_BUF];
	    
	int key = ftok(".", 1);    
	int sem = semget(key, 2, 0666 | IPC_CREAT);    
    struct sembuf wait = {1, 0, SEM_UNDO};
	struct sembuf lock = {1, 1, SEM_UNDO};
	struct sembuf unlock = {0, -1, SEM_UNDO};
	
	int fd[2];
	if(pipe(fd) < 0){
		printf("Pipe error\n");
		exit(0);
	}
	
	switch(pid = fork()){
		case 0: 	close(fd[1]);
					_itoa(fd[0], buf);
					_itoa(key, key_buf);
					execl("client", " ", buf, key_buf, NULL);	
					break;
					
		case -1: 	printf("fork error\n");
					break;

		default:	close(fd[0]);
					printf("Вводите сообщения для отправки:\n");
					while(1){		
						semop(sem, &wait, 1);
						semop(sem, &lock, 1);
												
						fgets(str, SIZE_BUF, stdin);
						write(fd[1], str, SIZE_BUF);
						
						semop(sem, &unlock, 1);
					}	
				    break;
	}	
	return 0;
}
