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
#define DATA_OFFSET 16896
#define FIRST_LOGICAL_CLUSTER_OFFSET 26
#define FAT_NUMBER_OFFSET 16
#define SECTORS_PER_FAT_OFFSET 22

const char *osName(char *p);
const char *diskLabel(char *p);
int diskSize(char *p);
int freeSize(char *p, int size);
int fileAmount(char *p);
int fatAmount(char *p);
int sectorsPerFat(char *p);

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
	const char *os = osName(p);
	printf("OS Name: %s\n", os);

	// Get label of the disk from Root Directory
	const char *label = diskLabel(p);
	printf("Label of the disk: %s\n", label);

	// Get size of disk
	printf("Total size of the disk: %lu bytes\n", (uint64_t)sb.st_size);

	int free_count = freeSize(p, (uint64_t)sb.st_size);
	printf("Free size of the disk: %d bytes\n", free_count);

	printf("==============\n");

	int file_amount = fileAmount(p);
	printf("The number of files in the image: %d\n", file_amount);

	printf("==============\n");

	int fat_amount = fatAmount(p);
	printf("Number of FAT copies: %d\n", fat_amount);

	int sectors_per_fat = sectorsPerFat(p);
	printf("Sectors per FAT: %d\n", sectors_per_fat);
	
	munmap(p, sb.st_size);
	close(fd);

	return 1;
}

const char *osName(char *p){
	char *os = malloc( 8 * sizeof(char) );
	for(int i = 0; i < OS_SIZE; i++){
		os[i] = p[OS_OFFSET + i];
	}
	os[OS_SIZE] = '\0';

	return os;
}

const char *diskLabel(char *p){
	char *label = malloc( LABEL_SIZE * (sizeof(char)) );
	for(int i = ROOT_DIRECTORY; i <= DATA_OFFSET; i += DIRECTORY_SIZE){
		// Check if volume label of Attribute is set
		if(CHECK_BIT(p[i + ATTRIBUTE_OFFSET], 3) && p[i + ATTRIBUTE_OFFSET] != 0x0F){
			// Copy Filename into label string
			for(int j = 0; j < LABEL_SIZE - 1; j++){
				label[j] = p[i + j];
			}
			break;
		}
	}

	return label;
}

int diskSize(char *p){
	return 0;
}

int freeSize(char *p, int size){
	int free_count = 0;
	int n = 0;
	int odd, even;
	// Get free size of disk
	// Iterates through each sector in data area
	for(int i = 0; i <= (size/SECTOR_SIZE - 32); i++){
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

	return free_count;
}

int fileAmount(char *p){
	int count = 0;
	for(int i = ROOT_DIRECTORY; i <= DATA_OFFSET; i += DIRECTORY_SIZE){
		// Check if volume label of Attribute is set
		if(CHECK_BIT(p[i + ATTRIBUTE_OFFSET], 3) 
			|| p[i + ATTRIBUTE_OFFSET] == 0x0F
			|| p[i] == 0xE5
			|| p[i] == 0x004
			|| (p[i + FIRST_LOGICAL_CLUSTER_OFFSET] == 0 && (p[i + FIRST_LOGICAL_CLUSTER_OFFSET + 1] == 0 || p[i + FIRST_LOGICAL_CLUSTER_OFFSET + 1] == 1))
		){
			continue;
		}
		count++;
	}

	return count;
}

int fatAmount(char *p){
	return p[FAT_NUMBER_OFFSET];
}

int sectorsPerFat(char *p){
	return p[SECTORS_PER_FAT_OFFSET];
}
