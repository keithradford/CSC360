#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

int main(int argc, char *argv[]){
	if(argc != 2){
		printf("Error: disklist usage must be \n./disklist <DISK>\n");
		return 0;
	}

	printf("disklist\n%s\n", argv[1]);

	return 1;
}
