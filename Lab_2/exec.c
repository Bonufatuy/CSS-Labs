#include <signal.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>

int flag = 0;

void awake(int sig){
	flag = 1;
}

void waiting(){
	while(flag == 0){}
	flag = 0;
}

void removeProcess(int sig){
	exit(0); 
}

void main(){

	signal(SIGUSR1, awake);
	signal(SIGUSR2, removeProcess);	
	kill(getppid(), SIGUSR1);

	while(1){		
		waiting();
		printf("myPid: %d\n", getpid());
		kill(getppid(), SIGUSR1);
	}
}
