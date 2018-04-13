#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>

#define OPTION_i 0
#define OPTION_l 1
#define OPTION_n 2
#define OPTION_c 3
#define OPTION_w 4
#define OPTION_r 5

void getDirTree(const char *dir_path, int num_args, char** args, bool* options) {
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
        getDirTree(path, num_args, args, options);
      }else{
        waitpid(pid,NULL,0);
      }

    } else {
      //printf("%s\n", path);
      checkFile(path, num_args, args, options);
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

bool* getOptions(int num_args, char* args[], bool* options) {
  // get options
  if(num_args > 2) {
    int c=1;
    while(c < num_args-2) {
      printf("%s\n", args[c]);
      if(strcmp(args[c], "-i") == 0) {options[OPTION_i] = true; printf("-i true\n");}
      else if(strcmp(args[c], "-l") == 0) {options[OPTION_l] = true; printf("-l true\n");}
      else if(strcmp(args[c], "-n") == 0) {options[OPTION_n] = true; printf("-n true\n");}
      else if(strcmp(args[c], "-c") == 0) {options[OPTION_c] = true; printf("-c true\n");}
      else if(strcmp(args[c], "-w") == 0) {options[OPTION_w] = true; printf("-w true\n");}
      else if(strcmp(args[c], "-r") == 0) {options[OPTION_r] = true; printf("-r true\n");}
      c++;
    }
  }

  printf(options[OPTION_i] ? "option_i = true\n" : "option_i = false\n");
  printf(options[OPTION_l] ? "option_l = true\n" : "option_l = false\n");
  printf(options[OPTION_n] ? "option_n = true\n" : "option_n = false\n");
  printf(options[OPTION_c] ? "option_c = true\n" : "option_c = false\n");
  printf(options[OPTION_w] ? "option_w = true\n" : "option_w = false\n");
  printf(options[OPTION_r] ? "option_r = true\n" : "option_r = false\n\n");

  return;
}

// finds pattern according to options
char* findOptions(char* line, char* pattern, bool* options) {
  if(options[OPTION_i] && options[OPTION_w]) return;

  if(options[OPTION_i]) return strstr(strupr(line), strupr(pattern));

  if(options[OPTION_w]) return;

  return strstr(line, pattern);
}

// decides what to print according to options
void printOptions(char* path, char* line, int num_line, bool* options) {
  if(options[OPTION_c]) return;

  else if(options[OPTION_l]) printf("Found in file %s\n\n", path);

  else if(options[OPTION_n]) printf("%s in line %d:\n%s\n", path, num_line, line);

  else printf("%s:\n%s\n", path, line);
}

void checkFile(char* path, int num_args, char* args[], bool* options) {
  FILE * fp;
  char * line = NULL;
  size_t lenght = 0;
  ssize_t read;
  char* pattern = NULL;
  bool l_complete = false;

  fp = fopen(path, "r");

  if (fp == NULL){
    return;
  }

  pattern = args[num_args-2];

  printf("Checking file %s for %s\n\n", path, pattern);

  int line_num = 1;
  while ((read = getline(&line, &lenght, fp)) != -1) {
    if(findOptions(&line, pattern, options) != NULL){
      l_complete = true;
      printOptions(path, line_num, line, options);
    }
    line_num++;
    // with option_l it only needs to find the pattern once in the file
    if(l_complete) break;
  }

  if(options[OPTION_c]) printf("%s: found in %d lines\n", path, line_num);

  printf("Done checking file %s for %s\n\n", path, pattern);

  fclose(fp);
}

int main(int argc, char* argv[]) {
  if(argv[1] == NULL){
    printf("ERROR: wrong number of arguments\n Try [options] <pattern> <filepath>\n");
    return;
  }

  if(argc < 2 || argc > 7){
    printf("ERROR: wrong number of arguments\n Try [options] <pattern> <filepath>\n");
    return;
  }

  bool options[6] = {false, false, false, false, false, false};

  getOptions(argc, argv, &options);

  // if option_r then check the directory, else check the file asked for
  if(options[OPTION_r]) getDirTree(".", argc, argv, options);
  else checkFile(argv[argc-1], argc, argv, options);

  return 0;
}
