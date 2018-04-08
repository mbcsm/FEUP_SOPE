#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

typedef enum { false, true } bool;

int main(int argc, char* argv[]) {
  if(argv[1] == NULL){
    printf("ERROR: wrong number of arguments\n Try [options] <pattern> <filepath>\n");
    return;

  }
  getDirTree(".", argc, argv);
  return 0;
}

void getDirTree(const char *dir_path, int num_args, char** args) {
  DIR *dir;
  struct dirent *name;

  if (!(dir = opendir(dir_path)))
  return;

  while ((name = readdir(dir)) != NULL) {

    char path[2048];
    strcpy(path, dir_path);
    strcat(path, "/");
    strcat(path, name->d_name);

    if (name->d_type == DT_DIR) {

      if (strcmp(name->d_name, ".") == 0 || strcmp(name->d_name, "..") == 0)
      continue;

      snprintf(path, sizeof(path), "%s/%s", dir_path, name->d_name);

      //printf("%d    -----   [%s]\n",  getpid(), name->d_name);

      int pid= fork();
      if(pid < 0) {
        printf("ERROR: child killed at birth\n");
      }

      else if (pid ==  0) {
        getDirTree(path, num_args, args);
      }else{
        waitpid(pid,NULL,0);
      }

    } else {
      //printf("%s\n", path);
      checkFile(path, num_args, args);
      //printf("%d    -----    %s\n",  getpid(), name->d_name);
    }
  }

  closedir(dir);
}

char *strupr(char *str) {
  unsigned char *p = (unsigned char *)str;

  while (*p) {
     *p = toupper((unsigned char)*p);
      p++;
  }

  return str;
}

// finds pattern according to options
char* findOptions(char* line, char* string, bool option_i, bool option_w) {
  if(option_i && option_w) return;

  if(option_i) return strstr(strupr(line), strupr(string));

  if(option_w) return;

  return strstr(line, string);
}

// decides what to print according to options
void printOptions(char* path, char* line, int num_line, bool option_l, bool option_n, bool option_c) {
  if(option_c) return;

  else if(option_l) printf("%s", path);

  else if(option_n) printf("%s in line %d:\n%s\n", path, num_line, line);

  else printf("%s:\n%s\n", path, line);
}

void checkFile(char* path, int num_args, char* args[]) {
  FILE * fp;
  char * line = NULL;
  size_t lenght = 0;
  ssize_t read;
  char* string = NULL;
  bool option_i=false, option_l=false, option_n=false, option_c=false, option_w=false, option_r=false;

  fp = fopen(path, "r");

  if (fp == NULL){
    return;
  }

  // get options
  if(num_args > 2) {
    int c=1;
    while(c < num_args-1) {
      if(args[c] == "-i") option_i = true;
      else if(args[c] == "-l") option_l = true;
      else if(args[c] == "-n") option_n = true;
      else if(args[c] == "-c") option_c = true;
      else if(args[c] == "-w") option_w = true;
      else if(args[c] == "-r") option_r = true;
      c++;
    }
  }

  string = args[num_args-1];

  int line_num = 1;
  while ((read = getline(&line, &lenght, fp)) != -1) {
    if(findOptions(&line, string, option_i, option_w) != NULL){
      printOptions(path, line_num, line, option_l, option_n, option_c);
    }
    line_num++;
  }

  if(option_c) printf("%s: found in %d lines\n", path, line_num);

  fclose(fp);
}
