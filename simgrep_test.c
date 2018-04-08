#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#define BUF_LENGTH 256
#define MAX 8

char** searchFileFor(FILE* src, char* pattern, int num_lines) {
  char** Lines[10]; // putting a cap on number of times we spot a pattern, for now
  num_lines = 0;
  int file_line=1; // this is for one of the options

  char * line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, src)) != -1) {
    char* tmp = strstr(line, pattern);
    if(tmp != NULL) {
      Lines[num_lines] = malloc(41 * sizeof(char));
      strncpy(Lines[num_lines], tmp, 40);

      printf("Line %d: %s\n", file_line, Lines[num_lines]);
      num_lines++;
    }

    if(num_lines >= 10) break;

    file_line++;
  }

  return Lines;
}

void main(int argc, char* argv[]) {

  if(argc < 3) {
    perror("ERROR: wrong arguments input. Try [options] <pattern> <file/dir>\n");
    exit(2);
  }

  char* pattern = argv[argc-2];

  FILE *src;
  char buf[BUF_LENGTH];
  char *file = argv[argc-1];

  printf("Looking for %s in %s\n\n", pattern, file);

  // open file
  if((src = fopen(file, "r")) == NULL) {
    printf("Error number %d - no such file\n", errno);
    exit(4);
  }

  // TODO -read line by line
  int num_lines=0;
  char** Lines = searchFileFor(src, pattern, &num_lines);

  // printing
  int i;
  for(i=0; i < num_lines; i++) {
    printf("%s\n", Lines[i]);
  }


  exit(1);
}
