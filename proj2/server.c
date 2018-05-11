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
#include <time.h>
#include "header.h"
#include <semaphore.h>


pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int req,
		num_room_seats,
		num_ticket_offices,
		open_time,
		total_requests = 0,
		total_requests_responded = 0;

const char *fifo_name = "requests";

struct Seat {
	int clientId;
  int taken;
  int seat;
  pthread_mutex_t mtx;
} seat[1000];

struct Request {
	int seatsWanted[MAX_CLI_SEATS];
	int numSeatsWanted;
} request[1000];

void closeFIFO() {
  printf(".. cleaning up files ..\n");
	close(req);
  remove(fifo_name);
  exit(2);
}

int * isSeatFree(int seatNum) {
		if(seat[seatNum].taken == 1){
			return 1;
		}
	return 0;
}

void bookSeat(int seatNum, int clientId){
	seat[seatNum].clientId = clientId;
	seat[seatNum].taken = 1;
}

void freeSeat(int seatNum){
	seat[seatNum].taken = 0;

}

void *createThreadForTicketBooth(void *arg){
  unsigned long i = 0;
  pthread_t threadId = pthread_self();


  while(true){
		pthread_mutex_lock(&lock);
		pthread_cond_wait(&cv, &lock);
		int requestToCheck = total_requests_responded;
		total_requests_responded++;
		pthread_mutex_unlock(&lock);

		printf("%d - %d - ", threadId, request[requestToCheck].numSeatsWanted);
		for(int i = 0; i < request[requestToCheck].numSeatsWanted; i++ ){
			printf("%d ", request[requestToCheck].seatsWanted[i]);
		}

		int seatNotFree = 0;
		for(int i = 0; i < request[requestToCheck].numSeatsWanted; i++ ){
			if(seatNotFree == 1)
				continue;
			pthread_mutex_lock(&seat[request[requestToCheck].seatsWanted[i]].mtx);
			if(isSeatFree(request[requestToCheck].seatsWanted[i])==0)
				bookSeat(request[requestToCheck].seatsWanted[i], requestToCheck);
			else
				seatNotFree = 1;
			pthread_mutex_unlock(&seat[request[requestToCheck].seatsWanted[i]].mtx);

			if(seatNotFree == 1){
				for(int j = 0; j < i; j++ ){
					pthread_mutex_lock(&seat[request[requestToCheck].seatsWanted[j]].mtx);
					freeSeat(request[requestToCheck].seatsWanted[j]);
					pthread_mutex_unlock(&seat[request[requestToCheck].seatsWanted[j]].mtx);
					i = 999;
				}
				printf(" - NAV \n");
				//Send Message To Client Saying the Seats are already taken
			}
		}
		if(seatNotFree == 0){
			printf(" - ");
			for(int i = 0; i < request[requestToCheck].numSeatsWanted; i++ ){
				printf("%d ", request[requestToCheck].seatsWanted[i]);
			}
			printf("\n");
		}


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
	/*
  char cmd[1024];
	printf("Creating Requests.....\n");

	srand(time(NULL));
	for(int i = 0; i < 100; i ++){
		total_requests++;
		request[i].numSeatsWanted = rand() % MAX_CLI_SEATS + 1;
		for(int j = 0; j < request[i].numSeatsWanted; j ++){
			request[i].seatsWanted[j] = rand() % (999 + 1 - 0) + 0;
		}
	}
	*/


	/*
	printf("Press 'send' to send a request (repeat if you want to send more)\n");
	while(fscanf(stdin, "%s", cmd) != EOF) {
		if(strcmp(cmd, "send") == 0) {
			pthread_cond_signal(&cv);
			sleep(1);
		}

	}
	*/


	//reading from FIFO
	int nread;
	char buf[256], request_pid[10], request_numseats[10], request_seats[100];

	req = mkfifo(fifo_name, 0660);
	if(req != 0)
		perror("\n Request FIFO make");

  req = open(fifo_name, O_RDONLY);
		perror("\n Request FIFO open");

  int iteration = 0;
	while(1) {
		memset(buf, 0, 256);

    if(readRequests(buf)) {
			memset(request_pid, 0, 10);
			memset(request_numseats, 0, 10);
			memset(request_seats, 0, 100);

			if(!(sscanf(buf, "PID: %s NumSeats: %s Seats: %s", request_pid, request_numseats, request_seats)) || atoi(request_pid) == 0) {
				//printf("\n Invalid request\n");
			}

			else {
				printf("\n Request received: %s\n", buf);

				// extracting request
				int req_pid, req_numseats, req_seats[10];
				int c, bytesread, seat_arr_c = 0;
				char* tmp_seat_list = request_seats;

				while(sscanf(tmp_seat_list, "%d%n", &c, &bytesread) > 0) {
					req_seats[seat_arr_c++] = c;
					tmp_seat_list += bytesread;
				}

				req_pid = atoi(request_pid);
				req_numseats = atoi(request_numseats);

				// send request to a channel

			}
		}

    iteration++;
		printf("\n Waiting for requests..\n");
    sleep(3);
	}
}

int readRequests(char *str) {
  int n;
  do {
    n = read(req,str,1);
  } while(n>0 && *str++ != '\0');

  return (n>0);
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

	atexit(closeFIFO);
  signal(SIGINT, closeFIFO);

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
