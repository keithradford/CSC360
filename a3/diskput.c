#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#define CHECK_BIT(var, pos) ((var) & (1<<(pos)))

#define ACCESS_DATE_OFFSET 18
#define ATTRIBUTE_OFFSET 11
#define DATA_OFFSET 16896
#define DATE_OFFSET 16
#define DIRECTORY_SIZE 32
#define EXTENSION_OFFSET 8
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
#define WRITE_DATE_OFFSET 24
#define WRITE_TIME_OFFSET 22

int findSubdirLocation(char *p, char *subdir);
const char *getFileName(char *p, int start, int directory);
char* setDirectoryEntry(char *p, char* file, int start, struct stat sb);

char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sept", "Oct", "Nov", "Dec"};


int main(int argc, char *argv[]){
	int fd, to_put;
	struct stat sb;

	if(argc != 3){
		printf("Error: diskput usage must be \n./diskput <DISK> <FILE>\n");
		return 0;
	}

	char *disk = malloc ( strlen(argv[1]) * sizeof(char) );;
	strcpy(disk, argv[1]);	

	fd = open(disk, O_RDWR);
	fstat(fd, &sb);

	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}

	char s[2] = "/";
	char* token;
	char* file;
	char* subdir;
	char* path = malloc( strlen(argv[2]) * sizeof(char) );
	char* tmp = malloc( strlen(argv[2]) * sizeof(char) );
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
	path[m - n] = '\0';

	strcpy(tmp, path);

	token = strtok(tmp, s);
	while(token != NULL){
		subdir = malloc( strlen(token) * sizeof(char) );
		strcpy(subdir, token);
		token = strtok(NULL, s);
	}

	printf("subdir last %s\n", subdir);

	int subdir_location = findSubdirLocation(p, subdir);
	printf("location: %d\n", subdir_location);

	struct stat stats;
	to_put = open(file, O_RDWR);
	fstat(to_put, &stats);

	setDirectoryEntry(p, file, subdir_location, stats);

	return 1;
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
		// printf("yo %c\n", p[start + j]);
	}
	name[j] = '\0';

	if(directory)
		return name;

	int i = 0;
	for(int k = 8; k < 11; k++){
		// printf("%c\n", p[start + k]);
		extension[i] = p[start + k];
		i++;
	}

	// printf("extension %s\n", extension);

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

	// printf("%s\n", file_name);

	return file_name;
}

int findSubdirLocation(char *p, char *subdir){
	int found = 0;
	for(int i = ROOT_DIRECTORY; i < 1474560; i += DIRECTORY_SIZE){
		unsigned int high = p[i + FIRST_LOGICAL_CLUSTER_OFFSET + 1] << 8;
		unsigned int low = p[i + FIRST_LOGICAL_CLUSTER_OFFSET];
		unsigned int first_logical_cluster = high + low;
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
		const char *curr = getFileName(p, i, 1);
		// printf("%s\n", curr);
		if(strcmp(curr, subdir) == 0){
			printf("Matched: %s and %s\n", curr, subdir);
			return (first_logical_cluster + 31) * 512;
		}
	}
}

int findFreeSector(char *p){
	for(int i = DATA_OFFSET; i < 1474560; i++){

	}
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

int diskSize(char *p){
	int high = p[SECTOR_OFFSET + 1] << 8;
	int low = p[SECTOR_OFFSET];
	int sector_count = high + low;

	return sector_count * 512;
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

int findFreeEntries(char *p, int size){
	// double amnt = (double)size/512;
	// printf("%d\n", (int)ceil(amnt));
	int count = 0;
	int good = 0;
	int j = 0;
	int disk_size = diskSize(p);
	// int n = 0;
	int odd, even;
	// Get free size of disk
	// Iterates through each sector in data area
	int n = 0;
	for(n = 33; n < (disk_size/SECTOR_SIZE); n++){

		if(getFatEntry(p, n) == 0x00){
			// printf("hey %d\n", getFatEntry(p, n));
			for(j = 0; j < size; j++){
				printf("@ %d = %d\n", (n+j), getFatEntry(p, n + j));
				if(getFatEntry(p, n + j) != 0x00)
					break;
			}
			// printf("j = %d, amnt = %d\n", j, amnt);
			if(j == size)
				break;
		}
	}

	return n - 31;
}

void putFatEntries(char *p, int n, int amnt){
	// int n = size - 31;
	int size = diskSize(p);

	int odd = 1 + (3*n)/2;
	int even = (3*n)/2;
	int a = p[512 + odd] & 0x000000FF;
	int b = p[512 + even] & 0x000000FF;
	// If n is even, first 4 bits are in next byte
	// Remaining 8 bits are in current byte
	if(n % 2 == 0){
		for(int i = 0; i < amnt; i++){
			if(i == (size - 1)){
				p[512 + i + odd] = ((0xFF9 << 12) & 0x000000FF) + p[512 + i + odd];
				p[512 + i + even] = 0xFF9 & 0x000000FF;
			}
			else{
				p[512 + i + odd] = (((n + 32) << 12) & 0x000000FF) + p[512 + i + odd];
				p[512 + i + even] = (n + 32) & 0x000000FF;
			}
		}
		// return (a << 12) + b;
	}
	// If n is odd, first 4 bits are in current byte
	// Remaining 8 bits are in next byte
	else{
		for(int i = 0; i < amnt; i++){
			if(i == (size - 1)){
				p[512 + i + even] = ((0xFF9 << 12) & 0x000000FF) + p[512 + i + even];
				p[512 + i + odd] = 0xFF9 & 0x000000FF;
			}
			else{
				p[512 + i + even] = (((n + 32) << 12) & 0x000000FF) + p[512 + i + even];
				p[512 + i + odd] = (n + 32) & 0x000000FF;
			}
		}
		//return (b << 12) + a;
	}

	return;
}

void writeData(char *p, const char *file, int start, int size){
	FILE *fp;
    char ch;
    int i = 0;

    //2
    printf("%d\n", start);
    fp = fopen(file, "r");

    //3
    if (fp == NULL)
    {
        printf("File is not available \n");
    }
    else
    {
        //4
        while ((ch = fgetc(fp)) != EOF)
        {
            // printf("%c", ch);
            p[start + i] = ch;
            i++;
        }
    }

    //5
    fclose(fp);

    return;

}

char* setDirectoryEntry(char *p, char* file, int start, struct stat sb){
	int i = 0;
	for(i = start; i < (start + 512); i += DIRECTORY_SIZE){
		if(p[i] == 0x00){
			break;
		}
	}

	printf("%d\n", i);
	char s[2] = ".";
	char* token;
	token = strtok(file, s);
	printf("%s\n", token);
	for(int j = 0; j < 8; j++){
		if(j >= strlen(token))
			p[j + i] = 0x20;
		else
			p[j + i] = token[j];
	}

	token = strtok(NULL, s);
	printf("%s\n", token);
	for(int j = 0; j < strlen(token); j++){
		if(j >= strlen(token))
			p[j + i] = 0x20;
		else
			p[i + j + EXTENSION_OFFSET] = token[j];
	}

	printf("file name %s\n", getFileName(p, i, 0));
	const char *file_name = getFileName(p, i, 0);

	// printf("%c %c %c\n", p[start + EXTENSION_OFFSET], p[start + EXTENSION_OFFSET + 1], p[start + EXTENSION_OFFSET + 2]);

    if (sb.st_mode & W_OK)
        p[start + ATTRIBUTE_OFFSET] = 0x01;

	//put attribute at offset 11


    // dt = *(ctime(&sb.st_mtime));

    // time_t rawtime;
	struct tm *dt, *access;

	dt = localtime ( &sb.st_mtime );
	access = localtime (&sb.st_atime);

    // printf("\nCreated on: %d-%d-%d %d:%d:%d\n", dt.tm_mday, dt.tm_mon, dt.tm_year + 1900, dt.tm_hour, dt.tm_min, dt.tm_sec);

    int year = (dt->tm_year - 80) << 9;
    int month = (dt->tm_mon + 1) << 5;
    int day = dt->tm_mday;

    uint16_t date = year + month + day;

    uint8_t  bytes[2];

	bytes[0] = *((uint8_t*)&(date)+1);
	bytes[1] = *((uint8_t*)&(date)+0);

	p[i + DATE_OFFSET] = bytes[1];
	p[i + DATE_OFFSET + 1] = bytes[0];

	p[i + WRITE_DATE_OFFSET] = bytes[1];
	p[i + WRITE_DATE_OFFSET + 1] = bytes[0];

	int hour = (dt->tm_hour) << 11;
    int minutes = (dt->tm_min) << 5;

    uint16_t time = hour + minutes;

    bytes[0] = *((uint8_t*)&(time)+1);
	bytes[1] = *((uint8_t*)&(time)+0);

	p[i + TIME_OFFSET] = bytes[1];
	p[i + TIME_OFFSET + 1] = bytes[0];

	p[i + WRITE_TIME_OFFSET] = bytes[1];
	p[i + WRITE_TIME_OFFSET + 1] = bytes[0];

	printFileDate(p, i);
	printf("\n");

	//last access date
	year = (access->tm_year - 80) << 9;
    month = (access->tm_mon + 1) << 5;
    day = access->tm_mday;

	date = year + month + day;

    bytes[2];

	bytes[0] = *((uint8_t*)&(date)+1);
	bytes[1] = *((uint8_t*)&(date)+0);

	p[i + ACCESS_DATE_OFFSET] = bytes[1];
	p[i + ACCESS_DATE_OFFSET + 1] = bytes[0];

	printf("\nFile size: %ld\n", sb.st_size);

	double amnt = (double)sb.st_size/512;
	// printf("%d\n", (int)ceil(amnt));

	int fat_start = findFreeEntries(p, (int)ceil(amnt));
	putFatEntries(p, fat_start, (int)ceil(amnt));
	printf("%d\n", fat_start);

	int sector = (fat_start + 31);
	writeData(p, file_name, sector * 512, sb.st_size);

	// first logical cluster 26

	uint16_t first_logical_cluster = fat_start;

	bytes[0] = *((uint8_t*)&(first_logical_cluster)+1);
	bytes[1] = *((uint8_t*)&(first_logical_cluster)+0);

	p[i + FIRST_LOGICAL_CLUSTER_OFFSET] = bytes[1];
	p[i + FIRST_LOGICAL_CLUSTER_OFFSET + 1] = bytes[0];

	uint32_t size = sb.st_size;

	uint8_t four_bytes[4];

	four_bytes[0] = *((uint8_t*)&(size)+3);
	four_bytes[1] = *((uint8_t*)&(size)+2);
	four_bytes[2] = *((uint8_t*)&(size)+1);
	four_bytes[3] = *((uint8_t*)&(size));

	p[i + FILE_SIZE_OFFSET] = four_bytes[3];
	p[i + FILE_SIZE_OFFSET + 1] = four_bytes[2];
	p[i + FILE_SIZE_OFFSET + 2] = four_bytes[1];
	p[i + FILE_SIZE_OFFSET + 3] = four_bytes[0];
	printf("%d %x %x %x %x\n", size,four_bytes[3], four_bytes[2], four_bytes[1], four_bytes[0]);


	// file size 28
}
