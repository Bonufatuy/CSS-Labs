#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int flag = 0;

void waiting(){
	while(flag == 0){}
	flag = 0;	
}

void func(int sig){
	flag = 1;
}

int main(int argc, char* argv[]){
	signal(SIGUSR1, func);
	
	char msg[64]; 
	int fd = atoi(argv[1]);
	
	while(1){
		waiting();
		read(fd, msg, 64);
		printf("taken message: %s\n", msg);
		kill(getppid(), SIGUSR1);
	}
	
	exit(0);
}
