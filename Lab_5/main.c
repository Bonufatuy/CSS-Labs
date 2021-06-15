#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>

#define BUF_SIZE 512

int main(){
    void* lib;  
           
    void *writer;
    void *reader; 
    
    lib = dlopen("./libfunc.so", RTLD_LAZY);        
	
	writer = dlsym(lib, "writer"); 
	reader = dlsym(lib, "reader"); 
	
	int hndl[2];                                                              

    if(pipe(hndl)){
        printf("Failed to create pipe!\n");
        return -1;
    }
	
	pthread_t reader_thread;         
    if(pthread_create(&reader_thread, NULL, reader, &hndl[1])){
       printf("Failed to create reader_Thread\n");
       return -1;
    }
	
	pthread_t writer_thread;         
    if(pthread_create(&writer_thread, NULL, writer, &hndl[0])){
        printf("Failed to create writer_thread\n");
        pthread_cancel(reader_thread);
        return -1;
    }  
	
    pthread_join(reader_thread, NULL);                                               
    pthread_cancel(writer_thread);                                             
	                                                       
    dlclose(lib); 

    int size, size_buf;
    int file = open("output", O_RDONLY);
    
    char buf[BUF_SIZE];
    struct stat stat;  

    fstat(file, &stat);                                                   
    size = stat.st_size;  

    while(true){
        if(size > BUF_SIZE) 
            size_buf = BUF_SIZE;
        else 
            size_buf = size;

        read(file, buf, size_buf);
        printf("%s", buf);

        if(size > BUF_SIZE){
            size -= BUF_SIZE;
        }
        else break;  
    }

    return 0;
}
