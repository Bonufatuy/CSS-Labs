#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <stdbool.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <aio.h>
#include <errno.h>

#define BUF_SIZE 512
#define MAX_LENGHT_NAME_FILE 64
#define PATH_DIRECTORY_RESOURCES "./resources/"

bool isTxt(char* str){

    int i = 0;
    char search[] = "txt.";

    while(str[i+1]) i++;

    for(int j = 0; j < 4; j++)
		if(str[i-j] != search[j]) 
			return 0;

    return 1;
}

bool isValid(char* str){
    if(!isTxt(str)) 
        return 0;
    return 1;
}

void* writer (void* ptr){

    int pipe = *(int*)ptr;                                                    
	long int symRead;
    char buf[BUF_SIZE];                                                        
    
    int file;                                                                  
    file = open("output", O_WRONLY | O_TRUNC | O_CREAT, 0644);                   

    int sem = semget(ftok(".", 1), 0, IPC_CREAT | 0666);              
    struct sembuf wait = {0, 0, SEM_UNDO};                                     
    struct sembuf lock = {0, 1, SEM_UNDO};                                     
    struct sembuf unlock = {1, -1, SEM_UNDO};  

    struct aiocb writeFile;                                                          
    writeFile.aio_fildes = file;                                              
    writeFile.aio_offset = 0;                                                  
    writeFile.aio_buf = &buf;                                                  
	
	semop(sem, &lock, 1);
    while(true){
		
        semop(sem, &wait, 1);                                                  
        semop(sem, &lock, 1);
		        
        symRead = read(pipe, buf, BUF_SIZE);                                  

        if(!symRead) break;

        writeFile.aio_nbytes = symRead;     
                                       
        aio_write(&writeFile);                                                 
        while(aio_error(&writeFile) == EINPROGRESS);                  

        writeFile.aio_offset += symRead;                                       
        semop(sem, &unlock, 1);                                                
    } 
}

void* reader (void* ptr){

    int pipe = *(int*)ptr;                                                     
    char buf[BUF_SIZE];                                                        

    int sem = semget(ftok(".", 1), 2, IPC_CREAT | 0666);          
    struct sembuf wait = {1, 0, SEM_UNDO};                                     
    struct sembuf lock = {1, 1, SEM_UNDO};                                     
    struct sembuf unlock = {0, -1, SEM_UNDO};                              

    struct aiocb readFile;                                                            
    readFile.aio_buf = &buf;                                                   
	
    DIR* directory;                                                            
    struct dirent* nextFile;                                                          
    directory = opendir(PATH_DIRECTORY_RESOURCES);                               
	
    int file, size;                                                                 
    struct stat stat;                                                          
    
    while(true){
  
        nextFile = readdir(directory);                                         
        if(nextFile == NULL) break;   	                                   
        if(!isValid(nextFile->d_name)) continue;
        
		char filePath[MAX_LENGHT_NAME_FILE] = PATH_DIRECTORY_RESOURCES; 
		strcat(filePath, nextFile->d_name);
        file = open(filePath, O_RDONLY);                              

        fstat(file, &stat);                                                   
        size = stat.st_size;                                                   
		
        readFile.aio_fildes = file;                                            
        readFile.aio_offset = 0;                                               

        while(true){
        
            if(size > BUF_SIZE) readFile.aio_nbytes = BUF_SIZE;
            else readFile.aio_nbytes = size;

            aio_read(&readFile);                                              
            while(aio_error(&readFile) == EINPROGRESS);
            
            semop(sem, &wait, 1);                                            
            semop(sem, &lock, 1);
           	
            write(pipe, buf, readFile.aio_nbytes);                      
            semop(sem, &unlock, 1);                                            

            if(size > BUF_SIZE){
                size -= BUF_SIZE;
                readFile.aio_offset += BUF_SIZE;
            }
            else break;   
        }
        
        close(file);                                                           
    }

    semop(sem, &wait, 1);                                                    
}
