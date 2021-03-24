#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <linux/unistd.h>
#include <ncurses.h>

int main(int argc, int *argv[]){
	int pid = fork();
	if(pid == -1){
		printf("Error\n");
		return 0;
	}
	if(pid == 0){
		execl("exec", " ", NULL);
	}else if(pid > 0){
		time_t timer;
		while(1){
			sleep(1);
			timer = time(NULL);	
			printf("Parent process(%d), time: %s", getpid(), ctime(&timer));
		}
	}
	waitpid(pid);
	return 0;
}
