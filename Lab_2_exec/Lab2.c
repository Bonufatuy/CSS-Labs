#include <signal.h>
#include <sys/sem.h>
#include <ncurses.h>

int flag = 0;

void awake(int sig){
	flag = 1;
}

void waiting(){
	while(flag == 0){}
	flag = 0;
}

void removeProcess(int sig){
	exit(1);
}

void main(){

	initscr();
    cbreak();
    noecho();
    scrollok(stdscr, TRUE);
    nodelay(stdscr, TRUE);

	signal(SIGUSR1, awake);
	signal(SIGUSR2, removeProcess);

	pid_t pid;
	pid_t chPids[64] = {0};
	const pid_t mainPid = getpid();
	int num = 0;
	char act;

	while(1){
	
		for(int i = 0; i < num; i++){
			kill(chPids[i], SIGUSR1);
			waiting();
		}
		sleep(1);
		clear();	
		
		act = getch();
		switch(act){
			case '+': 		pid = fork(2);
							if(pid == 0){
								execl("exec", " ", NULL);
							}else if(pid > 0){
								chPids[num] = pid;
								num++;
								waiting();				
							}
							
							break;
			
			case '-':		if(num > 0){
								kill(chPids[num - 1], SIGUSR2);
								chPids[num - 1] = 0;
								num--;
							} 
							
							break;
							
			case 'q':	    for(int i = 0; i < num; i++){
								kill(chPids[i], SIGUSR2);
							}
							exit(0); 
							
							break;
		}
	
	}
	endwin();
	
	for(int i = 0; i < num; i++){
		kill(chPids[i], SIGUSR2);
		waitpid(chPids[i]);
	}
}
