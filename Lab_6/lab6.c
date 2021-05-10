#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h> 

#define MAX_AMOUNT_OF_MEMORY 70
#define MAX_ELEMENTS_SWAP_FILE 10
#define SIZE_DATA_STRUCT sizeof(struct data_struct)

int used_memory = 0;
int amount_elements_Swap_file = 0;

int has_initialized = 0;
void *managed_memory_start;
void *last_valid_address;


struct data_struct {
	int available;
	int size;
	int refcount;
	void** address;
};

struct info_about_Swap_element{
 	void** ptr;
 	int size;
} elements_in_swap_file[MAX_ELEMENTS_SWAP_FILE];

void swapping();

void malloc_init() {
	last_valid_address = sbrk(0);
	managed_memory_start = last_valid_address;
 	has_initialized = 1;
}

void my_free(void *firstbyte) {
	struct data_struct *mcb;
	mcb = (struct data_struct*) (firstbyte - SIZE_DATA_STRUCT);
	mcb->available = 1;
	mcb->refcount = 0;
	return;
}

void *my_malloc(long numbytes) {
	void *current_location;
	void *memory_location;
	struct data_struct *current_location_mcb;

	if(! has_initialized) 	{
		malloc_init();
	}

	numbytes = numbytes + SIZE_DATA_STRUCT;

	if(used_memory+numbytes > MAX_AMOUNT_OF_MEMORY){
		swapping();
	}

	memory_location = 0;
	current_location = managed_memory_start;

	while(current_location != last_valid_address) {

		current_location_mcb = (struct data_struct *)current_location;

		if(current_location_mcb->available) {
			if(current_location_mcb->size >= numbytes) {

				current_location_mcb->available = 0;
				memory_location = current_location;

				used_memory += (numbytes - SIZE_DATA_STRUCT);

				break;
			}
		}

		current_location = current_location + current_location_mcb->size;
	}

	if(! memory_location) {

		sbrk(numbytes);
		memory_location = last_valid_address;

		last_valid_address = last_valid_address + numbytes;

		current_location_mcb 			= (struct data_struct *) memory_location;
		current_location_mcb->available = 0;
		current_location_mcb->size 		= numbytes;
		current_location_mcb->refcount  = 0;

		used_memory += numbytes;
	}

	memory_location = memory_location + SIZE_DATA_STRUCT;

	return memory_location;
 }
											
void swapping(){
	int file = open("swapfile.bin", O_WRONLY | O_APPEND | O_CREAT, 0644);

	struct data_struct *current_location_mcb;	
	void *current_location = managed_memory_start;

	while(current_location != last_valid_address && amount_elements_Swap_file < MAX_ELEMENTS_SWAP_FILE){

		current_location_mcb = (struct data_struct *)current_location;
		current_location = current_location + SIZE_DATA_STRUCT;

		used_memory -= (current_location_mcb->size - SIZE_DATA_STRUCT);
		
		elements_in_swap_file[amount_elements_Swap_file].ptr = current_location_mcb->address;
		elements_in_swap_file[amount_elements_Swap_file].size = current_location_mcb->size - SIZE_DATA_STRUCT;
		

		write(file, &(elements_in_swap_file[amount_elements_Swap_file]), sizeof(struct info_about_Swap_element));
		write(file, current_location, current_location_mcb->size - SIZE_DATA_STRUCT);

		amount_elements_Swap_file++;

		my_free(current_location);

		current_location = current_location + current_location_mcb->size - SIZE_DATA_STRUCT;
	}

	close(file);
}

 int find_memory_RAM_or_ROM(void** ptr){
	for(int i = 0; i < amount_elements_Swap_file; i++){
		if(elements_in_swap_file[i].ptr == ptr)
			return i;
	}
	return -1;
}

void return_Element_ROM(void** ptr){
	int number_of_find_element = find_memory_RAM_or_ROM(ptr);
	
	if(number_of_find_element != -1){
		struct info_about_Swap_element* found_element = &elements_in_swap_file[number_of_find_element];

		void* current_location;
		struct info_about_Swap_element info_about_Swap_element;

		struct stat stat;

		const char* s;
		
		int file = open("swapfile.bin", O_RDONLY);

		if(file){
			fstat(file, &stat);  
			int size = 0;

			while(size < stat.st_size){
				size += read(file, &info_about_Swap_element, sizeof(struct info_about_Swap_element));

				if(info_about_Swap_element.ptr == found_element->ptr){

					current_location = my_malloc(info_about_Swap_element.size);
					*ptr = current_location;
					read(file, current_location, info_about_Swap_element.size);
					close(file);
					return;
				}

				size += info_about_Swap_element.size;
				lseek(file, info_about_Swap_element.size, SEEK_CUR);

			}
			close(file);
		}	
	}
	return;
}

void REF(void **data){
	return_Element_ROM(data);

	struct data_struct *data_struct;
	data_struct = (struct data_struct *) (*data - SIZE_DATA_STRUCT);
	data_struct->refcount++;
	data_struct->address = data;
}

void UNREF(void *data){
	struct data_struct *data_struct;
	data_struct = (struct data_struct *) (data - SIZE_DATA_STRUCT);
	data_struct->refcount--;
}

int main(){

	int* Integer_10 = (int*)my_malloc(sizeof(int) * 10);
	REF(&Integer_10);
	for(int i = 0; i < 10; i++){
	 	Integer_10[i] = i;
	 	printf("Первый массив : %d\n", Integer_10[i]);
	}
	UNREF(Integer_10);

	printf("\n");

	int* Integer_20 = (int*)my_malloc(sizeof(int) * 10);
	REF(&Integer_20);
	for(int i = 0; i < 10; i++){
	 	Integer_20[i] = i+10;
	 	printf("Второй массив : %d\n", Integer_20[i]);
	}
	UNREF(Integer_20);

	printf("\n");

	REF(&Integer_10);
	for(int i = 0; i < 10; i++){
		printf("Первый массив: %d\n", Integer_10[i]);
	}
	UNREF(Integer_10);

	printf("\n");

	REF(&Integer_20);
	for(int i = 0; i < 10; i++){
		printf("Второй массив: %d\n", Integer_20[i]);
	}
	UNREF(Integer_20);

	remove("swapfile.bin");
	return 0;
}