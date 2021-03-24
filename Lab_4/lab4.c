#include <pthread.h>
#include <sys/sem.h>
#include <ncurses.h>

#define amount_pthread 10

struct pthrData{
	int number;
	key_t key_for_pthread;
};

union semun{
	int val;
	struct semid* buff;
	unsigned short *array;
	struct seminfo* buf;
};

void* threadFunc(void* thread_data){
	struct pthrData* data = thread_data;
	struct sembuf unlock = {data->number, 1, 0};
	int semid = semget(data->key_for_pthread, 0, 0666 | IPC_CREAT);
	
	while(1){
		if(semctl(semid, data->number, GETVAL, 0) == 0){
			printw("pthread number = %d\n", data->number);
			semop(semid, &unlock, 1);
		}
	}
	return NULL;
}

int main(){
	initscr();
    cbreak();
    noecho();
    scrollok(stdscr, TRUE);
    nodelay(stdscr, TRUE);	
	
	pthread_t tids[amount_pthread]; 
	
	union semun arg;
	struct pthrData thread_data[amount_pthread];
	
	struct sembuf changes[amount_pthread*2];	
	for(int i = 0, j = 1, k = 0; k < amount_pthread; i++, j+=2, k++){
		changes[i].sem_num = k;											
		changes[i].sem_op = -1;											
		changes[i].sem_flg = 0; 										
	}
	
	key_t key = ftok(".", 'S');
	int semid = semget(key, amount_pthread, 0666 | IPC_CREAT);
	
	for(int i = 0; i < amount_pthread; i++){
		thread_data[i].number = i;
		thread_data[i].key_for_pthread = key;
	}	
	
	arg.val = 1;
	for(int i = 0; i < amount_pthread; i++)
		semctl(semid, i, SETVAL, arg);
	
	int number = 0; 
    char act; 
	
    while(1){		
    	for(int i = 0; i < number; i++){
			semop(semid, &changes[i], 1);
			while(semctl(semid, i, GETVAL, 0) == 0);
    	}
    	printw("\n");
    				
	    act = getch();
		switch(act){
			case '+': 		pthread_create(&(tids[number]), NULL, threadFunc, &thread_data[number]);
							number++;
							break;
			
			case '-':		if(number > 0){
								pthread_cancel(tids[number-1]);
								number--;
							}
							break;
							
			case 'q':	    for(int i = 0; i < number; i++)
								pthread_cancel(tids[i]);
							semctl(semid, 0, IPC_RMID, 0);
							return 0;
		}
		sleep(1);
	}
	
	semctl(semid, 0, IPC_RMID, 0);
	endwin();
	
    return 0;
}


int _strlen(char* s) {
	int i;
	i = 0;
	while (s[i] != '\0')
		++i;
	return i;
}

void _reverse(char* s){
	int i, j;
	char h;
	for (i = 0, j = _strlen(s)-1; i < j; i++, j--) {
 		h = s[i];
		s[i] = s[j];
		s[j] = h;
	}
}

void _itoa (int n, char* s) {
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

void _strcat(char* s, char* t, char* sym){
	int i, j;
	i = j = 0;
	while (s[i] != '\0') sym[i++] = s[i];
	while (t[j] != '\0') sym[i++] = t[j++];
}
