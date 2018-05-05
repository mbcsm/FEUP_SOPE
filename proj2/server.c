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
	

pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
  
int num_room_seats,
		num_ticket_offices,
		open_time,
		total_requests = 0,
		total_requests_responded = 0;

struct Seats {
  int taken;
  int seat;
  pthread_mutex_t mtx;
} seat[1000];

struct Requests {
  char requestInfo[256] ;
} request[1000];


void *createThreadForTicketBooth(void *arg){
	printf("L.\n");
  unsigned long i = 0;
  pthread_t threadId = pthread_self();

  
  while(true){
		printf("Locking and waiting. Type unlock to unlock me.\n");
		pthread_mutex_lock(&lock);
		pthread_cond_wait(&cv, &lock);
		printf("I've been unlocked - %d.\n", threadId);
		pthread_mutex_unlock(&lock);
		sleep(1);
  }
	
  

  return NULL;
}

void * receiveClientRequests(void * arg) {
  //opening booths threads
	pthread_t tid[num_ticket_offices];
	
	printf("\n Openning [%d] Booths\n", num_ticket_offices);
	for(int i = 0; i < num_ticket_offices; i++){
			int err;
			err = pthread_create(&(tid[i]), NULL, createThreadForTicketBooth, NULL);
			if (err != 0){
				printf("\ncan't create thread :[%s]", strerror(err));
				return NULL;
			}
			else{
				printf("\n Booth Openned successfully\n");
			}
	}
	
	
	
	//test manual unlock of booths, for testing purposes only!!!!!
  char cmd[1024];
	printf("Type lock to run a thread that locks and waits.\n");
	printf("Type unlock to unlock the same thread.\n");
	while(fscanf(stdin, "%s", cmd) != EOF) {
		if(strcmp(cmd, "lock") == 0) { 
		} else if(strcmp(cmd, "unlock") == 0) {
			pthread_cond_signal(&cv);
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
   		total_requests++;
   		
    }
		
    iteration++;
    sleep(1);
	}
	*/
}

int initializeSeats(){
	for(int i = 0; i< num_room_seats; i++){
		seat[i].taken = 0;
		seat[i].seat = i;
		seat[i].mtx = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;;
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
	
	
	initializeSeats();
	
	
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
