#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include "header.h"
#include <semaphore.h>
	
	
volatile int* array[10000];
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
  
int num_room_seats,
		num_ticket_offices,
		open_time;
	
	
struct Seats {
  int seat;
  int mtx;
} seat;

struct Requests {
  char requestInfo[256] ;
} request;


void *createThreadForTicketBooth(void *pointer){
  unsigned long i = 0;
  pthread_t id = pthread_self();
  
  pthread_mutex_lock(&mtx);
  
	pthread_mutex_unlock(&mtx);

  return NULL;
}


void * receiveClientRequests(void * arg) {
	
  struct Seats seat[num_room_seats];
  struct Requests request;
  
  //opening booths threads
	pthread_t tid[num_ticket_offices];
	
	printf("\n Openning [%d] Booths\n", num_ticket_offices);
	for(int i = 0; i < num_ticket_offices; i++){
		  int i = 0;
			int err;
			err = pthread_create(&(tid[i]), NULL, createThreadForTicketBooth, NULL);
			if (err != 0){
				printf("\ncan't create thread :[%s]", strerror(err));
				return -1;
			}
			else{
				printf("\n Booth Openned successfully\n");
    		pthread_join(tid[i], NULL); 
			}
	}
	
	
	//reading from FIFO
	int fd, nread;
  const char *fifo_name = "requests";
  char buf[256] ;
  fd = open(fifo_name, O_RDONLY);
  
  int iteration = 0;
	while(1) {
	
    memset(buf, 0, 256);
    
    if(read(fd, buf, 256)>0){
   		printf("%d - %s\n", iteration, buf);
   		
    }
		
    iteration++;
    sleep(1);
	}
}


int main(int argc, char* argv[]) {
  if(argc != 4){
    printf("ERROR: wrong number of arguments\n Try [options] <pattern> <filepath>\n");
    return -1;
  }
  
	num_room_seats = atoi(argv[1]);
	num_ticket_offices = atoi(argv[2]);
	open_time = atoi(argv[3]);
	
	
	
	printf("\n Openning Store\n");
	pthread_t mainThreadId;
  int err;
	err = pthread_create(&mainThreadId, NULL, receiveClientRequests, NULL);
	if (err != 0){
		printf("\ncan't create thread :[%s]", strerror(err));
		return -1;
	}
	else{
		printf("\n Store Opened\n");
		pthread_join(mainThreadId, NULL); 
	}   
	  

	
  return 0;
}
