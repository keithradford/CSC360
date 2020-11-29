#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "queue.h"

#define CHECK_BIT(var, pos) ((var) & (1<<(pos)))

#define ATTRIBUTE_OFFSET 11
#define DATA_OFFSET 16896
#define DATE_OFFSET 16
#define DIRECTORY_SIZE 32
#define EXTENSION_SIZE 3
#define FAT_NUMBER_OFFSET 1
#define FIRST_LOGICAL_CLUSTER_OFFSET 26
#define FILE_SIZE_OFFSET 28
#define LABEL_SIZE 9
#define NAME_SIZE 9
#define OS_OFFSET 3
#define OS_SIZE 8
#define ROOT_DIRECTORY 9728
#define SECTOR_OFFSET 19
#define SECTOR_SIZE 512
#define SECTORS_PER_FAT_OFFSET 22
#define TIME_OFFSET 14

int getFatEntry(char *p, int n);
const char *getFileName(char *p, int start, int directory);
int getFileSize(char *p, int start);
const char *getFileDate(char *p, int start);
void printFiles(char *p, int start, int root, char *parent);
int diskSize(char *p);

struct Queue* directories = NULL;

char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sept", "Oct", "Nov", "Dec"};

int main(int argc, char *argv[]){
	int fd;
	struct stat sb;
	char *disk = malloc ( strlen(argv[1]) * sizeof(char) );

	directories = createQueue(256);

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

	printFiles(p, ROOT_DIRECTORY, 0, "");
	// for(int i = ROOT_DIRECTORY; i < DATA_OFFSET; i += DIRECTORY_SIZE){
	// 	int high = p[i + FIRST_LOGICAL_CLUSTER_OFFSET + 1] << 8;
	// 	int low = p[i + FIRST_LOGICAL_CLUSTER_OFFSET];
	// 	int first_logical_cluster = high + low;
	// 	// Check if volume label of Attribute is set
	// 	if(
	// 		CHECK_BIT(p[i + ATTRIBUTE_OFFSET], 3) 
	// 		|| p[i + ATTRIBUTE_OFFSET] == 0x0F
	// 		|| (p[i] & 0xFF) == 0xE5
	// 		|| (p[i] & 0xFF) == 0x00
	// 		|| first_logical_cluster == 0
	// 		|| first_logical_cluster == 1
	// 	){
	// 		continue;
	// 	}
	// 	if(CHECK_BIT(p[i + ATTRIBUTE_OFFSET], 4))
	// 		printf("D\n");
	// 	else
	// 		printf("F\n");
	// }

	return 1;
}

int diskSize(char *p){
	int high = p[SECTOR_OFFSET + 1] << 8;
	int low = p[SECTOR_OFFSET];
	int sector_count = high + low;

	return sector_count * 512;
}

int getFatEntry(char *p, int n){
	n = n - 33 + 2;
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

const char *getFileName(char *p, int start, int directory){
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

	if(directory){
		for(int k = 0; k < strlen(name); k++){
			file_name[k] = name[k];
		}
		return file_name;
	}

	int n = 0;
	int k;
	for(k = 0; k < 12; k++){
		if(k < strlen(name)){
			file_name[k] = name[k];
		}
		else if((k > strlen(name))){
			if(n > 3 || extension[n] == ' ')
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

int getFileSize(char *p, int start){
	int fourth = (p[start + FILE_SIZE_OFFSET + 3] & 0xFF) << 24;
	int third = (p[start + FILE_SIZE_OFFSET + 2] & 0xFF) << 16;
	int second = (p[start + FILE_SIZE_OFFSET + 1] & 0xFF) << 8;
	int first = p[start + FILE_SIZE_OFFSET] & 0xFF;
	int file_size = first + third + second + first;

	return file_size;
}

void printFileDate(char *p, int start){
	int time, time_low, time_high, date, date_low, date_high;
	int hours, minutes, day, month, year;
	
	time_low = p[start + TIME_OFFSET];
	time_high = p[start + TIME_OFFSET + 1] << 8;
	time = time_low + time_high;

	date_low = p[start + DATE_OFFSET];
	date_high = p[start + DATE_OFFSET + 1] << 8;
	date = date_low + date_high;
	
	//the year is stored as a value since 1980
	//the year is stored in the high seven bits
	year = ((date & 0xFE00) >> 9) + 1980;
	//the month is stored in the middle four bits
	month = (date & 0x1E0) >> 5;
	//the day is stored in the low five bits
	day = (date & 0x1F);
	if(!months[month - 1])
		return;
	
	printf("%s %2d %d ", months[month - 1], day, year);
	//the hours are stored in the high five bits
	hours = (time & 0xF800) >> 11;
	//the minutes are stored in the middle 6 bits
	minutes = (time & 0x7E0) >> 5;
	
	printf("%02d:%02d", hours, minutes);
	
	return;	
}

char *getPath(char *name, char *parent){
	int name_len = strlen(name);
	int parent_len = strlen(parent);
	int len = name_len + parent_len + 1;
	char *to_ret = malloc(sizeof(char) * len);
	int j = 0;

	if(!parent_len)
		return name;

	for(int i = 0; i < len; i++){
		if(i < parent_len)
			to_ret[i] = parent[i];
		else if(i > parent_len){
			to_ret[i] = name[j];
			j++;
		}
		else
			to_ret[i] = '/';
	}

	// printf("return: %s\n", to_ret);

	return to_ret;
}

void printFiles(char *p, int start, int directory, char *parent){
	int dir_count = 0;
	int bound = 0;
	int sector_number = 0;
	int size = 0;
	if(directory == 0){
		bound = DATA_OFFSET;
		printf("Root\n==================\n");
	}
	else if(directory == 1){
		sector_number = (start - 32)/SECTOR_SIZE;2;
		if((getFatEntry(p, sector_number) > 0xFF8) && (getFatEntry(p, sector_number) < 0xFFF)){
			bound = start + (SECTOR_SIZE * 2) - 32;
		}
		else{
			bound = start + SECTOR_SIZE - 32;
		}
	}
	else{
		sector_number = (start - 64)/SECTOR_SIZE;2;
		if((getFatEntry(p, sector_number) > 0xFF8) && (getFatEntry(p, sector_number) < 0xFFF)){
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
			size = 0;
			const char *name = getFileName(p, i, 1);
			printf("D %10d %15s ", size, name);
			printFileDate(p, i);
			printf("\n");
			int sector = (33 + first_logical_cluster - 2) * 512;

			// printf("eq");
			enqueue(directories, parent, name, sector);

			// dir_count++;
		}
		else{
			const char *name = getFileName(p, i, 0);
			size = getFileSize(p, i);
			printf("F %10d %15s ", size, name);
			printFileDate(p, i);
			printf("\n");
		}
	}

	struct Node n;
	while(!isEmpty(directories)){
		n = dequeue(directories);
		// printf("%s\n==================\n", n.name);
		if(directory == 0){
			// printf("Parent: %s, Name: %s\n", n.path, n.name);
			printf("\n/%s\n==================\n", n.name);
			printFiles(p, n.sector + 32, 1, n.name);
		}
		else{
			char *path = getPath(n.name, n.path);
			// printf("Parent: %s, Name: %s\n", n.path, n.name);
			printf("\n/%s\n==================\n", path);
			printFiles(p, n.sector + 64, 2, path);
		}
	}

	return;
}
