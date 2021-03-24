#include <signal.h>
#include <stdio.h>
#include <sys/sem.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char* argv[]){
	time_t timer;
	while(1){
		sleep(1);
		timer = time(NULL);
		printf("child process(%d), time: %s", getpid(), ctime(&timer));
	}	
	return 0;
}
