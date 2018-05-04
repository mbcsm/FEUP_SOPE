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

int ansfifo;
char fifo_name[FIFO_LEN+1];

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

  char pid[WIDTH_PID];
  sprintf(pid, "%d", getpid());

  // Read args
  int timeout = atoi(argv[1]);
  int num_wanted_seats = atoi(argv[2]);
  char* pref_seat_list = argv[3];
  char* tmp_seat_list = pref_seat_list;
  int seat_arr[10];
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
  char msg[100];
  sprintf(msg, "PID: %s NumSeats: %d Seats: %s", pid, num_wanted_seats, pref_seat_list);
  printf("Request is '"); printf(msg); printf("'\n");

  // Create the FIFO that gets the answer from the server
  strcpy(fifo_name, "ans");
  strcat(fifo_name, pid);
  int ans;
  ansfifo = mkfifo(fifo_name, 0660);
  if(ansfifo != 0)
    perror("ansFIFO");

  printf("Waiting for answer from the server in FIFO %s..\n", fifo_name);
  signal(SIGALRM, alarm_handler);
  alarm(timeout);

  ans = open(fifo_name, O_RDONLY);

  char str[100];
  putchar('\n');
  while(readline(ans,str))
    printf("%s",str);

  // write to clog.txt

  exit(0);
}
