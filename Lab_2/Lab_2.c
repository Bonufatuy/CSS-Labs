#include <signal.h>
#include <stdio.h>
#include <sys/sem.h>
#include <ncurses.h>
#include <stdlib.h>

void awake(int sig);
void sleeping(int sig);
void waiting();

int flag = 0;

void main(){
	signal(SIGINT, awake);
	signal(SIGUSR1, sleeping); 

	const pid_t mainPid = getpid();
	pid_t chPids[64];
	int num = 0;

	char act;
	while(1){
		if(getpid() == mainPid){
			printf("Processes Info(%d):\n", num);
			for(int i = 0; i < num; i++){
				kill(chPids[i], SIGINT);
				waiting();
			}

			printf("----------------\n");
			printf("Processes manager:\n -add new process(+)\n -delete last process(-)\n -quit program\naction: ");	
			scanf("%c", &act);
			system("clear");
			while (getchar() != '\n');		

			switch(act){
				case '+':
					chPids[num] = fork();
					num++;
					break;
				case '-':
					if(num>0){
						printf("del\n");
						kill(chPids[num-1], SIGUSR1);
						chPids[num-1] = 0;
						num--;
					}
					break;
				case 'q':
					return 0;
				default:
					printf("error\n");
					break;
			}
		}else{
			waiting();
			printf("My word: %d\n", getpid());
			kill(mainPid, SIGINT);
		}
	}
	
	for(int i = 0; i < num; i++){
		kill(chPids[i], SIGUSR2);
		waitpid(chPids[i]);
	}
	
	exit(0);
}

void awake(int sig){
	flag = 1;
}
void sleeping(int sig){
	exit(1);
}

void waiting(){
	while(flag == 0){}
	flag = 0;
}
