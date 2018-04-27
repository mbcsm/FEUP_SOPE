#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>


int main(int argc, char* argv[]) {
	if(argv[1] == NULL){
		printf("ERROR: wrong number of arguments\n");
		return;
		
	}
	getDirTree("/", argv[1]);
	return 0;
}



void getDirTree(const char *dir_path, char* string)
{
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
				getDirTree(path, string);
			}else{
				waitpid(pid,NULL,0);
			}
			
		} else {
			//printf("%s\n", path);
			checkFile(path, string);
			//printf("%d    -----    %s\n",  getpid(), name->d_name);
		}
	}
	
	closedir(dir);
}

void checkFile(char* path, char* string){

	FILE * fp;
	char * line = NULL;
	size_t lenght = 0;
	ssize_t read;

	fp = fopen(path, "r");
	
	if (fp == NULL){
		return;
	}
	while ((read = getline(&line, &lenght, fp)) != -1) {
		if(strstr(line, string) != NULL){
			printf("%s - - - > %s", path, line);
		}
	}
	
	fclose(fp);
}























