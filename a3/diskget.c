#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

int main(int argc, char *argv[]){
	if(argc != 3){
		printf("Error: diskget usage must be \n./diskget <DISK> <FILE>\n");
		return 0;
	}

	printf("diskget\n%s\n%s\n", argv[1], argv[2]);

	return 1;
}
