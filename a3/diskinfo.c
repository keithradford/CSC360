#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

int main(int argc, char *argv[]){
	if(argc != 2){
		printf("Error: diskinfo usage must be \n./diskinfo <DISK>\n");
		return 0;
	}
	
	printf("diskinfo\n%s\n", argv[1]);

	return 1;
}
