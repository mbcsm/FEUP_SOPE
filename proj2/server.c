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

int num_room_seats,
		num_ticket_offices,
		open_time;
	
int *createThreadForRequest(){

  unsigned long i = 0;
  pthread_t id = pthread_self();


  return 0;
}

int receiveClientRequests(){
	pthread_t tid[num_ticket_offices];
	
	int fd, nread;
  const char *fifo_name = "requests";
  char buf[256] ;
  fd = open(fifo_name, O_RDONLY);
  
  int iteration = 0;
	while(1) {
	
    memset(buf, 0, 256);
    
    if(read(fd, buf, 256)>0){
   		printf("%d - %s\n", iteration, buf);
		  int i = 0;
			int err;

			err = pthread_create(&(tid[i]), NULL, createThreadForRequest, NULL);
			if (err != 0)
				printf("\ncan't create thread :[%s]", strerror(err));
			else
				printf("\n Thread created successfully\n");
    }
		
    iteration++;
    sleep(1);
	}
}

int main(int argc, char* argv[]) {
	printf("%d\n", argc);
  if(argc != 4){
    printf("ERROR: wrong number of arguments\n Try [options] <pattern> <filepath>\n");
    return -1;
  }
  
	num_room_seats = atoi(argv[1]);
	num_ticket_offices = atoi(argv[2]);
	open_time = atoi(argv[3]);
	
        
	receiveClientRequests();

	
  return 0;
}
