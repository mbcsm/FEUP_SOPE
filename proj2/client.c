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

  // read args
  int timeout = atoi(argv[1]);
  int num_wanted_seats = atoi(argv[2]);
  char* pref_seat_list = argv[3];
  int seat_arr[num_wanted_seats];
  int c, bytesread, seat_arr_c = 0;

  // debugging
  printf("TIMEOUT: %d\n", timeout);
  printf("NUM_WANTED_SEATS: %d\n", num_wanted_seats);

  while(sscanf(pref_seat_list, "%d%n", &c, &bytesread) > 0) {
    seat_arr[seat_arr_c++] = c;
    pref_seat_list += bytesread;
    if(seat_arr_c >= num_wanted_seats) break;
  }

  // debugging
  seat_arr_c = 0;
  while(seat_arr_c < num_wanted_seats) {
    printf("SEAT_ARR: %d\n", seat_arr[seat_arr_c]);
    seat_arr_c++;
  }

  // create FIFOs
  char pid[WIDTH_PID];
  sprintf(pid, "%d", getpid());

  char fifo_name[FIFO_LEN+1];
  strncpy(fifo_name, "ans", 3);
  strncat(fifo_name, pid, 5);
  // debugging
  printf("FIFO: %s\n", fifo_name);

  mkfifo(fifo_name, 0660);
  int ans = open(fifo_name, O_RDONLY);

  mkfifo("requests", 0660);
  int req = open("requests", O_WRONLY);

  exit(0);
}
