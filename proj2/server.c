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
		booths_open = 0,
		open_time,
		totalSeatsTaken = 0;
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
	int numSeats;
	int pid;
	int requestId;
} request[1000];

void writeToSLOG(char* logText){
		FILE *f = fopen("slog.txt", "a");
		if (f == NULL)
		{
				printf("Error opening file!\n");
				return;
		}

		fprintf(f, "%s\n", logText);

		fclose(f);
		return;
}
void writeToSBOOK(){
		FILE *f = fopen("sbook.txt", "w");
		if (f == NULL)
		{
			printf("Error opening file!\n");
			return;
		}
		for(int i = 0; i < num_room_seats; i++ ){
			if(seat[i].taken == 1){
				if(i < 10)
					fprintf(f, "000%d\n", i);
				else if(i > 9 && i < 100)
					fprintf(f, "00%d\n", i);
				else if(i > 99 && i < 1000)
					fprintf(f, "0%d\n", i);
				else
					fprintf(f, "%d\n", i);
			}
		}
		fclose(f);
		return;
}

void closeFIFO() {
	writeToSBOOK();
  printf(".. cleaning up files ..\n");
	close(req);
  remove(fifo_name);
  exit(2);
}

int * isSeatFree(int seatNum) {
	return seat[seatNum].taken;
}

void bookSeat(int seatNum, int clientId){
	seat[seatNum].clientId = clientId;
	seat[seatNum].taken = 1;
	pthread_mutex_lock(&lock);
	totalSeatsTaken++;
	pthread_mutex_unlock(&lock);
}

void freeSeat(int seatNum){
	seat[seatNum].taken = 0;
	pthread_mutex_lock(&lock);
	totalSeatsTaken--;
	pthread_mutex_unlock(&lock);
}

void *createThreadForTicketBooth(void *arg){
  int boothNumber = *((int *) arg);
  unsigned long i = 0;
  pthread_t threadId = pthread_self();

	char boothText[10000];
	snprintf(boothText, sizeof boothText, "%d - OPEN", boothNumber);
	writeToSLOG(boothText);

	printf("%d - OPEN\n", boothNumber);

  while(true){
		pthread_mutex_lock(&lock);
		pthread_cond_wait(&cv, &lock);
		int requestToCheck = total_requests_responded;
		total_requests_responded++;

		pthread_mutex_unlock(&lock);

		int error = 0;
		char logText[10000];
		//open fifo

		int fd;
		char ansfifo_name[FIFO_LEN+1];
		snprintf(ansfifo_name, sizeof ansfifo_name, "ans%d", request[requestToCheck].pid);
		mkfifo(ansfifo_name, 0666);

		printf("%d-%d-%d: ", boothNumber, request[requestToCheck].pid,request[requestToCheck].numSeatsWanted);
		snprintf(logText, sizeof logText, "%d-%d-%d: ", boothNumber, request[requestToCheck].pid,request[requestToCheck].numSeatsWanted);

		for(int i = 0; i < request[requestToCheck].numSeats; i++ ){
			printf("%d ", request[requestToCheck].seatsWanted[i]);

			int len = strlen(logText);
			snprintf(logText + len, (sizeof logText) - len, "%d ", request[requestToCheck].seatsWanted[i]);

			if(request[requestToCheck].seatsWanted[i] > num_room_seats || request[requestToCheck].seatsWanted[i] < 0){
				error = 2;
				fd = open(ansfifo_name, O_WRONLY);
				write(fd, "-2", sizeof("-2"));
				close(fd);
    		unlink(ansfifo_name);

				int len = strlen(logText);
				snprintf(logText + len, (sizeof logText) - len, " - NST");
				error = 1;
				writeToSLOG(logText);
				printf(" - NST \n");
				continue;
			}
		}


		if(request[requestToCheck].numSeatsWanted > MAX_CLI_SEATS && error == 0){
				fd = open(ansfifo_name, O_WRONLY);
				write(fd, "-1", sizeof("-1"));
				close(fd);
    		unlink(ansfifo_name);

				int len = strlen(logText);
				snprintf(logText + len, (sizeof logText) - len, " - MAX");
				error = 1;
				writeToSLOG(logText);
				printf(" - MAX \n");
				continue;
		}


		pthread_mutex_lock(&lock);
		if(num_room_seats - totalSeatsTaken < request[requestToCheck].numSeatsWanted && error == 0){

				fd = open(ansfifo_name, O_WRONLY);
				write(fd, "-6", sizeof("-6"));
				close(fd);
    		unlink(ansfifo_name);


				int len = strlen(logText);
				snprintf(logText + len, (sizeof logText) - len, " - FUL");

				error = 1;
				printf(" - FUL \n");
				writeToSLOG(logText);
				continue;
		}
		pthread_mutex_unlock(&lock);

		int totalSeatsReserved = 0;

		for(int i = 0; i < request[requestToCheck].numSeats; i++ ){
		
			if(request[requestToCheck].numSeatsWanted == totalSeatsReserved)
				continue;
				
			pthread_mutex_lock(&seat[request[requestToCheck].seatsWanted[i]].mtx);
			if(isSeatFree(request[requestToCheck].seatsWanted[i])==0){
				//printf("|%d-%d|", request[requestToCheck].seatsWanted[i], isSeatFree(request[requestToCheck].seatsWanted[i]));
				totalSeatsReserved++;
				bookSeat(request[requestToCheck].seatsWanted[i], request[requestToCheck].pid);
			}
			pthread_mutex_unlock(&seat[request[requestToCheck].seatsWanted[i]].mtx);
				
			
		}
		if(request[requestToCheck].numSeatsWanted != totalSeatsReserved){
			error = 1;
			for(int j = 0; j < request[requestToCheck].numSeats; j++ ){
					pthread_mutex_lock(&seat[request[requestToCheck].seatsWanted[j]].mtx);
					if(seat[request[requestToCheck].seatsWanted[j]].clientId == request[requestToCheck].pid)
						freeSeat(request[requestToCheck].seatsWanted[j]);
					pthread_mutex_unlock(&seat[request[requestToCheck].seatsWanted[j]].mtx);
				}


				fd = open(ansfifo_name, O_WRONLY);
				write(fd, "-5", sizeof("-5"));
				close(fd);
    		unlink(ansfifo_name);



				int len = strlen(logText);
				snprintf(logText + len, (sizeof logText) - len, " - NAV");

				printf(" - NAV \n");
				writeToSLOG(logText);
				continue;
				//Send Message To Client Saying the Seats are already taken
		}
		
		if(error == 0){
			printf(" - ");

			char seatsReserved[10000];
			int numSeatsReserved = 0;
			for(int j = 0; j < request[requestToCheck].numSeats; j++ ){
					pthread_mutex_lock(&seat[request[requestToCheck].seatsWanted[j]].mtx);
					if(seat[request[requestToCheck].seatsWanted[j]].clientId == request[requestToCheck].pid){
						printf("%d ", request[requestToCheck].seatsWanted[j]);
						snprintf(seatsReserved, sizeof seatsReserved, "%d ", request[requestToCheck].seatsWanted[j]);
						numSeatsReserved++;
					}
					pthread_mutex_unlock(&seat[request[requestToCheck].seatsWanted[j]].mtx);
				
			}
			fd = open(ansfifo_name, O_WRONLY);
			char msg[1000];
			sprintf(msg, "NumSeats: %d Seats: %s", numSeatsReserved, seatsReserved);
			write(fd, msg, sizeof(msg));
			close(fd);
			unlink(ansfifo_name);
			printf("\n");
			writeToSLOG(logText);
		}
	}
		
	return NULL;
}


void * receiveClientRequests(void * arg) {
  //opening booths threads
	pthread_t tid[num_ticket_offices];

	printf("\n Openning [%d] Booths\n", num_ticket_offices);
	for(int i = 0; i < num_ticket_offices; i++){
			int err;
			int *arg = malloc(sizeof(*arg));
      *arg = i;
			err = pthread_create(&(tid[i]), NULL, createThreadForTicketBooth, arg);
			if (err != 0){
				printf("\ncan't create thread :[%s]", strerror(err));
				return NULL;
			}
			else{
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
	char buf[3000], request_pid[10], request_numseats[1000], request_seats[1000];

	req = mkfifo(fifo_name, 0660);
	if(req != 0)
		perror("\n Request FIFO make");

  req = open(fifo_name, O_RDONLY);
		perror("\n Request FIFO open");

  int iteration = 0;
	while(1) {
		memset(buf, 0, 3000);

   if(readRequests(buf)) {
			memset(request_pid, 0, 10);
			memset(request_numseats, 0, 1000);
			memset(request_seats, 0, 1000);

			if(!(sscanf(buf, "PID: %s NumSeats: %s Seats: %s", request_pid, request_numseats, request_seats)) || atoi(request_pid) == 0) {
				//printf("\n Invalid request\n");
			}

			else {
				//printf("\n Request received: %s\n", buf);
				// extracting request
				int req_pid, req_numseats, req_seats[10];
				int c, bytesread, seat_arr_c = 0;
				char* tmp_seat_list = request_seats;

				sscanf(buf, "PID: %s NumSeats: %s Seats: %s", request_pid, request_numseats, request_seats);

				char *p = buf;
				int it = 0, seatsWanted = 0;
				while (*p) {
						if (isdigit(*p)) {
							int val = strtol(p, &p, 10);
							switch(it){
								case 0:
									request[total_requests].pid = val;
									break;
								case 1:
									request[total_requests].numSeatsWanted = val;
									break;
								default:
									request[total_requests].seatsWanted[seatsWanted] = val;
									request[total_requests].numSeats = seatsWanted + 1;
									seatsWanted++;
									break;
							}
							it++;
						} else {
							p++;
						}
				}
				pthread_cond_signal(&cv);
				total_requests++;
			}
		}
    iteration++;
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
