#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>

typedef enum { false, true } bool;

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

bool* getOptions(int num_args, char* args[]) {
  bool option_i=false, option_l=false, option_n=false, option_c=false, option_w=false, option_r=false;

  // get options
  if(num_args > 2) {
    int c=1;
    while(c < num_args-2) {
      if(strcmp(args[c], "-i")) option_i = true;
      else if(strcmp(args[c], "-l")) option_l = true;
      else if(strcmp(args[c], "-n")) option_n = true;
      else if(strcmp(args[c], "-c")) option_c = true;
      else if(strcmp(args[c], "-w")) option_w = true;
      else if(strcmp(args[c], "-r")) option_r = true;
      c++;
    }
  }
  bool* options = (bool*) malloc(6 * sizeof(bool));

  options[0] = option_i;
  options[1] = option_l;
  options[2] = option_n;
  options[3] = option_c;
  options[4] = option_w;
  options[5] = option_r;

  printf(options[0] ? "option_i = true\n" : "option_i = false\n");
  printf(options[1] ? "option_l = true\n" : "option_l = false\n");
  printf(options[2] ? "option_n = true\n" : "option_n = false\n");
  printf(options[3] ? "option_c = true\n" : "option_c = false\n");
  printf(options[4] ? "option_w = true\n" : "option_w = false\n");
  printf(options[5] ? "option_r = true\n" : "option_r = false\n\n");

  return options;
}

// finds pattern according to options
char* findOptions(char* line, char* pattern, bool option_i, bool option_w) {
  if(option_i && option_w) return;

  if(option_i) return strstr(strupr(line), strupr(pattern));

  if(option_w) return;

  return strstr(line, pattern);
}

// decides what to print according to options
void printOptions(char* path, char* line, int num_line, bool option_l, bool option_n, bool option_c) {
  if(option_c) return;

  else if(option_l) printf("Found in file %s", path);

  else if(option_n) printf("%s in line %d:\n%s\n", path, num_line, line);

  else printf("%s:\n%s\n", path, line);
}

void checkFile(char* path, int num_args, char* args[], bool* options) {
  FILE * fp;
  char * line = NULL;
  size_t lenght = 0;
  ssize_t read;
  char* pattern = NULL;

  fp = fopen(path, "r");

  if (fp == NULL){
    return;
  }

  pattern = args[num_args-2];

  printf("Checking file %s for %s\n\n", path, pattern);

  int line_num = 1;
  while ((read = getline(&line, &lenght, fp)) != -1) {
    if(findOptions(&line, pattern, options[0], options[4]) != NULL){
      printOptions(path, line_num, line, options[1], options[2], options[3]);
    }
    line_num++;
  }

  if(options[3]) printf("%s: found in %d lines\n", path, line_num);

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

  bool* options = getOptions(argc, argv);

  // if option_r then check the directory, else check the file asked for
  if(options[5]) getDirTree(".", argc, argv, options);
  else checkFile(argv[argc-1], argc, argv, options);

  return 0;
}
