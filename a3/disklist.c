#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define CHECK_BIT(var, pos) ((var) & (1<<(pos)))

#define ATTRIBUTE_OFFSET 11
#define DATA_OFFSET 16896
#define DIRECTORY_SIZE 32
#define FIRST_LOGICAL_CLUSTER_OFFSET 26
#define ROOT_DIRECTORY 9728

int main(int argc, char *argv[]){
	int fd;
	struct stat sb;
	char *disk = malloc ( strlen(argv[1]) * sizeof(char) );

	if(argc != 2){
		printf("Error: disklist usage must be \n./disklist <DISK>\n");
		return 0;
	}

	strcpy(disk, argv[1]);	

	fd = open(disk, O_RDWR);
	fstat(fd, &sb);

	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}

	for(int i = ROOT_DIRECTORY; i < DATA_OFFSET; i += DIRECTORY_SIZE){
		int high = p[i + FIRST_LOGICAL_CLUSTER_OFFSET + 1] << 8;
		int low = p[i + FIRST_LOGICAL_CLUSTER_OFFSET];
		int first_logical_cluster = high + low;
		// Check if volume label of Attribute is set
		if(
			CHECK_BIT(p[i + ATTRIBUTE_OFFSET], 3) 
			|| p[i + ATTRIBUTE_OFFSET] == 0x0F
			|| (p[i] & 0xFF) == 0xE5
			|| (p[i] & 0xFF) == 0x00
			|| first_logical_cluster == 0
			|| first_logical_cluster == 1
		){
			continue;
		}
		if(CHECK_BIT(p[i + ATTRIBUTE_OFFSET], 4))
			printf("D\n");
		else
			printf("F\n");
	}

	return 1;
}

int fileAmount(char *p){
	int count = 0;

	return count;
}
