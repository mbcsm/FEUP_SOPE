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
#include "header.h"


int receiveClientRequests(){
	int fd, nread;
  const char *fifo_name = "/tmp/requests";
  char buf[256] ;
  fd = open(fifo_name, O_RDONLY);
  
  int iteration = 0;
	while(1) {
    memset(buf, 0, 256);
    nread = read(fd, buf, 256);
    printf("%d - %s\n", iteration);
    sleep(1);
	}
}

int main(int argc, char* argv[]) {
	printf("%d\n", argc);
  if(argc != 4){
    printf("ERROR: wrong number of arguments\n Try [options] <pattern> <filepath>\n");
    return -1;
  }
  
	char* num_room_seats = argv[1];
	char* num_ticket_offices = argv[2];
	char* open_time = argv[3];
	
	
	receiveClientRequests();

	
  return 0;
}