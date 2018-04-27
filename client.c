#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "header.h"

int main(int argc, char* argv[]) {
  /*
  if(argv[1] == NULL){
    printf("ERROR: wrong number of arguments\n Try [options] <pattern> <filepath>\n");
    return;
  }

  if(argc < 2 || argc > 7){
    printf("ERROR: wrong number of arguments\n Try [options] <pattern> <filepath>\n");
    return;
  }
  */

  char pid[WIDTH_PID];
  sprintf(pid, "%d", getpid());

  char fifo_name[FIFO_LEN+1];
  strncpy(fifo_name, "ans", 3);
  strncat(fifo_name, pid, 5);
  printf("FIFO: %s\n", fifo_name);

  mkfifo(fifo_name, 0660);
  int fd = open(fifo_name, 0660);

  return 0;
}
