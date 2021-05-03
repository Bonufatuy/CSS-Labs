#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <dlfcn.h>

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
    return 0;
}
