#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define CHECK_BIT(var, pos) (((var)>>(pos)) & 1)

#define ATTRIBUTE_OFFSET 11
#define DATA_OFFSET 16896
#define DIRECTORY_SIZE 32
#define EXTENSION_SIZE 3
#define FAT_NUMBER_OFFSET 16
#define FIRST_LOGICAL_CLUSTER_OFFSET 26
#define LABEL_SIZE 9
#define NAME_SIZE 9
#define OS_OFFSET 3
#define OS_SIZE 8
#define ROOT_DIRECTORY 9728
#define SECTOR_OFFSET 19
#define SECTOR_SIZE 512
#define SECTORS_PER_FAT_OFFSET 22

const char *osName(char *p);
const char *diskLabel(char *p);
int diskSize(char *p);
int freeSize(char *p, int size);
int fileAmount(char *p, int start, int root);
int fatAmount(char *p);
int sectorsPerFat(char *p);
const char *getFileName(char *p, int start);
int getFatEntry(char *p, int n);

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
	int total_size = diskSize(p);
	printf("Total size of the disk: %d bytes\n", total_size);

	int free_count = freeSize(p, total_size);
	printf("Free size of the disk: %d bytes\n", free_count);

	printf("==============\n");

	int file_amount = fileAmount(p, ROOT_DIRECTORY, 1);
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

int getFatEntry(char *p, int n){
	n = n - 31;
	int size = diskSize(p);

	int odd = 1 + (3*n)/2;
	int even = (3*n)/2;
	int a = p[512 + odd] & 0x000000FF;
	int b = p[512 + even] & 0x000000FF;
	// If n is even, first 4 bits are in next byte
	// Remaining 8 bits are in current byte
	if(n % 2 == 0){
		return (a << 12) + b;
	}
	// If n is odd, first 4 bits are in current byte
	// Remaining 8 bits are in next byte
	else{
		return (b << 12) + a;
	}
}

const char *getFileName(char *p, int start){
	char *name = malloc(NAME_SIZE * sizeof(char));
	char *extension = malloc(NAME_SIZE * sizeof(char));
	char *file_name = malloc((NAME_SIZE + EXTENSION_SIZE + 1) * sizeof(char));

	// Copy Filename into label string
	int j;
	for(j = 0; j < NAME_SIZE - 1; j++){
		if(p[start + j] == 0x20)
			break;
		name[j] = p[start + j];
	}
	name[j] = '\0';

	int i = 0;
	for(int k = 8; k < 11; k++){
		extension[i] = p[start + k];
		i++;
	}

	int n = 0;
	for(int k = 0; k < 12; k++){
		if(k < strlen(name)){
			file_name[k] = name[k];
		}
		else if(k > strlen(name)){
			if(n > 3)
				break;
			file_name[k] = extension[n];
			n++;
		}
		else{
			file_name[k] = '.';
		}
	}

	return file_name;
}

const char *osName(char *p){
	char *os = malloc( OS_SIZE * sizeof(char) );
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
	int high = p[SECTOR_OFFSET + 1] << 8;
	int low = p[SECTOR_OFFSET];
	int sector_count = high + low;

	return sector_count * 512;
}

int freeSize(char *p, int size){
	int free_count = 0;
	// int n = 0;
	int odd, even;
	// Get free size of disk
	// Iterates through each sector in data area
	for(int n = 33; n < (size/SECTOR_SIZE); n++){
		odd = 1 + (3*n)/2;
		even = (3*n)/2;
		int a = p[512 + odd] & 0x000000FF;
		int b = p[512 + even] & 0x000000FF;

		if(getFatEntry(p, n) == 0x00){
			free_count++;
		}
	}
	free_count *= SECTOR_SIZE;

	return free_count;
}

int fileAmount(char *p, int start, int directory){
	int count = 0;
	int bound = 0;
	int sector = 0;
	if(directory == 0){
		bound = DATA_OFFSET;
	}
	else if(directory == 1){
		sector = (start - 32)/SECTOR_SIZE;2;
		if((getFatEntry(p, sector) > 0xFF8) && (getFatEntry(p, sector) < 0xFFF)){
			bound = start + (SECTOR_SIZE * 2) - 32;
		}
		else{
			bound = start + SECTOR_SIZE - 32;
		}
	}
	else{
		sector = (start - 64)/SECTOR_SIZE;2;
		if((getFatEntry(p, sector) > 0xFF8) && (getFatEntry(p, sector) < 0xFFF)){
			bound = start + (SECTOR_SIZE * 2) - 64;
		}
		else{
			bound = start + SECTOR_SIZE - 64;
		}
	}
	for(int i = start; i < bound; i += DIRECTORY_SIZE){
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

		if(CHECK_BIT(p[i + ATTRIBUTE_OFFSET], 4)){
			// printf("Going into subdirectory %s\n", getFileName(p, i));
			int sector = (33 + first_logical_cluster - 2) * 512;
			// printf("Going into subdirectory %s @ %d\n", getFileName(p, i), sector);
			if(directory == 0)
				count += fileAmount(p, sector + DIRECTORY_SIZE, 1);
			else
				count += fileAmount(p, sector + (DIRECTORY_SIZE*2), 2);
		}
		else{
			count++;
		}
	}

	return count;
}

int fatAmount(char *p){
	return p[FAT_NUMBER_OFFSET];
}

int sectorsPerFat(char *p){
	return p[SECTORS_PER_FAT_OFFSET];
}
