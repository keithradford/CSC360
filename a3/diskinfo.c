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

#define OS_OFFSET 3
#define OS_SIZE 8
#define LABEL_SIZE 9
#define ROOT_DIRECTORY 9728
#define DIRECTORY_SIZE 32
#define ATTRIBUTE_OFFSET 11
#define SECTOR_SIZE 512

int main(int argc, char *argv[]){
	int fd;
	struct stat sb;
	char *disk = malloc ( strlen(argv[1]) * sizeof(char) );

	if(argc != 2){
		printf("Error: diskinfo usage must be \n./diskinfo <DISK>\n");
		exit(1);
	}
	
	strcpy(disk, argv[1]);	

	fd = open(disk, O_RDWR);
	fstat(fd, &sb);

	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}
	
	// Get the OS Name from Boot Sector
	char os[OS_SIZE];
	for(int i = 0; i < OS_SIZE; i++){
		os[i] = p[OS_OFFSET + i];
	}
	os[OS_SIZE] = '\0';
	printf("OS Name: %s\n", os);

	// Get label of the disk from Root Directory
	char label[LABEL_SIZE];
	for(int i = ROOT_DIRECTORY; i <= (uint64_t)sb.st_size; i += DIRECTORY_SIZE){
		// Check if volume label of Attribute is set
		if(CHECK_BIT(p[i + ATTRIBUTE_OFFSET], 3) && p[i + ATTRIBUTE_OFFSET] != 0x0F){
			// Copy Filename into label string
			for(int j = 0; j < LABEL_SIZE - 1; j++){
				label[j] = p[i + j];
			}
			break;
		}
	}
	printf("Label of the disk: %s\n", label);

	// Get size of disk
	printf("Total size of the disk: %lu bytes\n", (uint64_t)sb.st_size);

	int free_count = 0;
	int n = 0;
	int odd, even;
	// Get free size of disk
	// Iterates through each sector in data area
	for(int i = 0; i <= ((uint64_t)sb.st_size/SECTOR_SIZE - 32); i++){
		odd = 1 + (3*n)/2;
		even = (3*n)/2;
		int a = p[512 + odd] & 0x000000FF;
		int b = p[512 + even] & 0x000000FF;
		// If n is even, first 4 bits are in next byte
		// Remaining 8 bits are in current byte
		if((n % 2 == 0) && (a & 0x0F) + b == 0x00){
			free_count++;
		}
		// If n is odd, first 4 bits are in current byte
		// Remaining 8 bits are in next byte
		else if((n % 2 != 0) && (b & 0xF0) + a == 0x00){
			free_count++;
		}
		n++;
	}
	free_count *= SECTOR_SIZE;
	printf("Free size of the disk: %d bytes\n", free_count);
	
	munmap(p, sb.st_size);
	close(fd);

	return 1;
}
