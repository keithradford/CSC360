#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

int main(int argc, char *argv[]){
	if(argc != 3){
		printf("Error: diskput usage must be \n./diskput <DISK> <FILE>\n");
		return 0;
	}
	char s[2] = "/";
	char* token;
	char* file;
	char* path = malloc( strlen(argv[2]) * sizeof(char) );
	strcpy(path, argv[2]);
	token = strtok(argv[2], s);

	// Get file from end of path
	while(token != NULL){
		file = malloc( strlen(token) * sizeof(char) );
		strcpy(file, token);
		token = strtok(NULL, s);
	}

	// Remove file name from end of path
	int n = strlen(file);
	int m = strlen(path);
	path[m - n - 1] = '\0';

	printf("diskput\n%s\n%s\n%s\n", argv[1], path, file);

	return 1;
}
