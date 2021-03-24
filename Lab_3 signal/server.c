#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>

int flag = 0;

void waiting(){
	while(flag == 0){}
	flag = 0;	
}

void func(int sig){
	flag = 1;
}

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
    signal(SIGUSR1, func);
    
	pid_t pid, mainPid = getpid();
	int fd[2];
	char buff[32], str[32];
	
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
		execl("client", " ", buff, NULL);
	}else{
		close(fd[0]);
		while(1){
			printf("Введите слово: ");
			scanf("%s", &str);
			write(fd[1], str, 30);
			kill(pid, SIGUSR1);
		    waiting();	
        }	
	}

    exit(0);
}
