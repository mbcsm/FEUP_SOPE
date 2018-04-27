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

int main(int argc, char* argv[]) {
  if(argc < 4){
    printf("ERROR: wrong number of arguments\n Try ./client <time_out> <num_wanted_seats> <pref_seat_list>\n");
    exit(1);
  }

  // Read args
  int timeout = atoi(argv[1]);
  int num_wanted_seats = atoi(argv[2]);
  char* pref_seat_list = argv[3];
  char* tmp_seat_list = pref_seat_list;
  int seat_arr[num_wanted_seats];
  int c, bytesread, seat_arr_c = 0;

  // debugging
  printf("TIMEOUT: %d\n", timeout);
  printf("NUM_WANTED_SEATS: %d\n", num_wanted_seats);

  while(sscanf(tmp_seat_list, "%d%n", &c, &bytesread) > 0) {
    seat_arr[seat_arr_c++] = c;
    tmp_seat_list += bytesread;
    if(seat_arr_c >= num_wanted_seats) break;
  }

  // debugging
  seat_arr_c = 0;
  while(seat_arr_c < num_wanted_seats) {
    printf("SEAT_ARR: %d\n", seat_arr[seat_arr_c]);
    seat_arr_c++;
  }

  // Create FIFOs
  char pid[WIDTH_PID];
  sprintf(pid, "%d", getpid());
  char fifo_name[FIFO_LEN+1];
  strcpy(fifo_name, "/fifos/");
  strncat(fifo_name, "ans", 3);
  strncat(fifo_name, pid, 5);

  // debugging
  printf("FIFO: %s\n", fifo_name);

  int ans;
  mkfifo(fifo_name, 0660);
  printf("FIFO %s made\n", fifo_name);
  if((ans = open(fifo_name, O_RDONLY)) == -1) printf("FIFO failed to open\n");
  else printf("FIFO %d opened\n", ans);
  /*
  int req;
  do {
    req = open("requests", O_WRONLY);
    if(req==-1) sleep(1);
  } while(req == -1);
  */

  // Send request
  char msg[100];
  sprintf(msg, "PID: %s NumSeats: %d Seats: %s", pid, num_wanted_seats, pref_seat_list);
  printf("MESSAGE - "); printf(msg); printf("\n");

  exit(0);
}
