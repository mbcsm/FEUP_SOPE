#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include "header.h"

int ansfifo, clog, cbook;
char fifo_name[FIFO_LEN+1];
char pid[WIDTH_PID];

int readline(int fd, char *str) {
  int n;
  do {
    n = read(fd,str,1);
  } while(n>0 && *str++ != '\0');

  return (n>0);
}

void alarm_handler() {
  printf("Client timed out..\n");
  exit(3);
}

void closeFIFO() {
  printf(".. cleaning up files ..\n");
  close(ansfifo);
  remove(fifo_name);
  exit(2);
}

int main(int argc, char* argv[]) {
  if(argc < 4){
    printf("ERROR: wrong number of arguments\n Try ./client <time_out> <num_wanted_seats> <pref_seat_list>\n");
    exit(1);
  }

  int ansfifo;
  atexit(closeFIFO);
  signal(SIGINT, closeFIFO);

  sprintf(pid, "%d", getpid());

  // Read args
  int timeout = atoi(argv[1]);
  int num_wanted_seats = atoi(argv[2]);
  char* pref_seat_list = argv[3];
  char* tmp_seat_list = pref_seat_list;
  int seat_arr[1000];
  int c, bytesread, seat_arr_c = 0;

  // debugging
  printf("TIMEOUT: %d\n", timeout);
  printf("NUM_WANTED_SEATS: %d\n", num_wanted_seats);

  while(sscanf(tmp_seat_list, "%d%n", &c, &bytesread) > 0) {
    seat_arr[seat_arr_c++] = c;
    tmp_seat_list += bytesread;
  }

  // debugging
  int num_seats = seat_arr_c;
  seat_arr_c = 0;
  while(seat_arr_c < num_seats) {
    printf("SEATS_WANTED: %d\n", seat_arr[seat_arr_c]);
    seat_arr_c++;
  }

  // Write to main FIFO
  int req;
  do {
    printf("\nTrying to open the main FIFO\n");
    req = open("requests", O_WRONLY);
    if(req==-1) sleep(3);
  } while(req == -1);

  printf("Opened main FIFO.. sending request now\n");

  // Send request
  char msg[3000];
  sprintf(msg, "PID: %s NumSeats: %d Seats: %s", pid, num_wanted_seats, pref_seat_list);
  printf("Request is '"); printf(msg); printf("'\n");
  write(req, msg, sizeof(msg));

  // Create the FIFO that gets the answer from the server
  strcpy(fifo_name, "ans");
  strcat(fifo_name, pid);
  ansfifo = mkfifo(fifo_name, 0660);
  if(ansfifo != 0)
    perror("ansFIFO");

  printf("Waiting for answer from the server in FIFO %s..\n", fifo_name);
  signal(SIGALRM, alarm_handler);
  alarm(timeout);
  ansfifo = open(fifo_name, O_RDONLY);
  perror("ansfifo open");

  // write server answer to clog.txt
  clog = open("clog.txt", O_WRONLY | O_CREAT, 0644);
  cbook = open("cbook.txt", O_WRONLY | O_CREAT, 0644);
  
  char answer[3000];
  char answerNumSeats[10], answerSeats[100], answerErr[10];
  while(1) {
    memset(answer, 0 , 3000);
	
    if(readline(ansfifo, answer)) {
		memset(answerNumSeats, 0 , 10);
		memset(answerSeats, 0 , 100);
		
		if(!(sscanf(buf, "NumSeats: %s Seats: %s", answerNumSeats, answerSeats))) {
				//printf("\n Invalid request\n");
				memset(answerErr, 0, 10);
				sscanf(buf, "%s", answerErr);
				int err = atoi(answerErr);
				if(err < 0) invAnswer_handler(err);
			}
		else {
			sscanf(buf, "NumSeats: %s Seats: %s", answerNumSeats, answerSeats);
			
			// extract msg
			int nSeats = atoi(answerNumSeats);
			int seats_arr[100];
			int c, bytesread, seat_arr_c = 0;;
			char* tmp_seat_list = answerSeats;
			while(sscanf(tmp_seat_list, "%d%n", &c, &bytesread) > 0) {
				seats_arr[seat_arr_c++] = c;
				tmp_seat_list += bytesread;
			}
			
			// write to clog.txt
			counter = 1;
			char clog_msg[300];
			char cbook_msg[100];
			while(counter < nSeats) {
				sprintf(clog_msg, "%s %d.%d %d\n", pid, counter, nSeats, seats_arr[counter]);
				write(clog, clog_msg, sizeof(clog_msg));

				sprintf(cbook_msg, "%d\n", seats_arr[counter]);
				write(cbook, cbook_msg, sizeof(cbook_msg));

				counter++;
			}
		}
    }
  }
  exit(0);
}

/* se invalido, a resposta recebida ? um int
* -1: qnt de lugares pedidos ? maior que o MAX_CLI_SEATS
* -2: numero de identificadores dos lugares pretendidos nao ? valido
* -3: os identificadores dos lugares pretendidos nao sao validos
* -4: outros erros
* -5: pelos menos um dos lugares nao esta disponivel
* -6: sala cheia
*/
void invAnswer_handler(int err) {
	char clog_msg[30];

	if(err == -1) {
		sprintf(clog_msg, "%s MAX\n", pid);
		write(clog, clog_msg, sizeof(clog_msg));
	}
	else 	if(err == -2) {
		sprintf(clog_msg, "%s NST\n", pid);
		write(clog, clog_msg, sizeof(clog_msg));
	}
	else 	if(err == -3) {
		sprintf(clog_msg, "%s IID\n", pid);
		write(clog, clog_msg, sizeof(clog_msg));
	}
	else 	if(err == -4) {
		sprintf(clog_msg, "%s ERR\n", pid);
		write(clog, clog_msg, sizeof(clog_msg));
	}
	else 	if(err == -5) {
		sprintf(clog_msg, "%s NAV\n", pid);
		write(clog, clog_msg, sizeof(clog_msg));
	}
	else 	if(err == -6) {
		sprintf(clog_msg, "%s FUL\n", pid);
		write(clog, clog_msg, sizeof(clog_msg));
	}
}
