#include "utils.h"

// Function signatures
int findSubdirLocation(char *p, char *subdir);
char* setDirectoryEntry(char *p, char* file, int start, struct stat sb);

int main(int argc, char *argv[]){
	int fd, to_put;
	struct stat sb;

	if(argc != 3){
		printf("Error: diskput usage must be \n./diskput <DISK> <FILE>\n");
		return 0;
	}

	// Open the given disk and map into p
	char *disk = malloc (strlen(argv[1]) * sizeof(char));
	strcpy(disk, argv[1]);	

	fd = open(disk, O_RDWR);
	fstat(fd, &sb);

	// Map disk binary to string p
	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); 
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

	// Get last subdirectory in path
	token = strtok(tmp, s);
	while(token != NULL){
		subdir = malloc( strlen(token) * sizeof(char) );
		strcpy(subdir, token);
		token = strtok(NULL, s);
	}


	struct stat stats;
	to_put = open(file, O_RDWR);
	fstat(to_put, &stats);
	if(strcmp(subdir, "") == 0){
		setDirectoryEntry(p, file, ROOT_DIRECTORY, stats);
	}

	else{
		int subdir_location = findSubdirLocation(p, subdir);
		if(subdir_location == 0){
			printf("Directory not found.\n");
			exit(1);
		}
		setDirectoryEntry(p, file, subdir_location, stats);
	}

	return 1;
}

/*
 * Function: findSubdirLocations
 * Parameter(s):
 * p, a string containing all disk information.
 * subdir, a string containing the subdirectory to find location.
 *
 * Returns the offset byte of where the given subdirectory starts.
 * The offset byte returned is where the subdirectories 
 */
int findSubdirLocation(char *p, char *subdir){
	int found = 0;
	for(int i = ROOT_DIRECTORY; i < 1474560; i += DIRECTORY_SIZE){
		unsigned int high = p[i + FIRST_LOGICAL_CLUSTER_OFFSET + 1] << 8;
		unsigned int low = p[i + FIRST_LOGICAL_CLUSTER_OFFSET];
		unsigned int first_logical_cluster = high + low;
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
		if(strcmp(curr, subdir) == 0){
			return (first_logical_cluster + 31) * 512;
		}
	}

	return 0;
}

/*
 * Function: findFreeEntries
 * Parameter(s):
 * p, a string containing all disk information.
 * size, an integer containing the size of free space needed.
 *
 * Returns the first sector that has enough free space to fit the given size.
 * Parses the FAT table looking for empty entries.
 * Once an entry is found, the function iterates size times forward in the FAT table
 * to make sure there is enough space.
 */
int findFreeEntries(char *p, int size){
	int count = 0;
	int good = 0;
	int j = 0;
	int disk_size = diskSize(p);
	int odd, even;

	// Iterates through FAT Table
	int n = 0;
	for(n = 33; n < (disk_size/SECTOR_SIZE); n++){

		if(getFatEntry(p, n) == 0x00){
			for(j = 0; j < size; j++){
				if(getFatEntry(p, n + j) != 0x00)
					break;
			}
			if(j == size)
				break;
		}
	}

	return n - 31;
}

/*
 * Function: putFatEntries
 * Parameter(s):
 * p, a string containing all disk information.
 * n, the first FAT entry to put data in.
 * amnt, the amount of fat entries to put data in
 *
 * Puts 0xF99 in the FAT entry if it is the last sector in the file,
 * else puts the starting offset of the next sector in the FAT entry.
 */
void putFatEntries(char *p, int n, int amnt){
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
	}

	return;
}

/*
 * Function: writeData
 * Parameter(s):
 * p, a string containing all disk information.
 * file, the name of the file to write to.
 * start, the offset in the disk to write the file to
 * size, the size of the file
 *
 * Starting at the appropriate location in the data sector, the file is opnened
 * and character by character it is written to the disk.
 */
void writeData(char *p, const char *file, int start, int size){
	FILE *fp;
    char ch;
    int i = 0;

    fp = fopen(file, "r");

    if(fp == NULL){
        printf("File not found\n");
        exit(1);
    }
    else{
        while((ch = fgetc(fp)) != EOF){
            p[start + i] = ch;
            i++;
        }
    }

    fclose(fp);

    return;
}

/*
 * Function: setDirectoryEntry
 * Parameter(s):
 * p, a string containing all disk information.
 * file, the name of the file to write to.
 * start, the offset of the directory
 * sb, the stat struct of the file
 *
 * Writes all appropriate data for a files directory entry
 * such as date, file size, first logical cluster etc.
 */
char* setDirectoryEntry(char *p, char* file, int start, struct stat sb){
	// Find the first free spot in the directory
	int i = 0;
	for(i = start; i < (start + 512); i += DIRECTORY_SIZE){
		if(p[i] == 0x00){
			break;
		}
	}

	// Split the file name at the "." so it can write filename and extension seperate
	char s[2] = ".";
	char* token;
	token = strtok(file, s);
	// Write filename
	for(int j = 0; j < 8; j++){
		if(j >= strlen(token))
			p[j + i] = 0x20;
		else
			p[j + i] = token[j];
	}

	// Write extension
	token = strtok(NULL, s);
	for(int j = 0; j < strlen(token); j++){
		if(j >= strlen(token))
			p[j + i] = 0x20;
		else
			p[i + j + EXTENSION_OFFSET] = token[j];
	}

	const char *file_name = getFileName(p, i, 0);

	// If not read only, write to attribute byte
    if (sb.st_mode & W_OK)
        p[start + ATTRIBUTE_OFFSET] = 0x01;

	struct tm *dt, *access;

	dt = localtime ( &sb.st_mtime );
	access = localtime (&sb.st_atime);

	// Form date into appropriate hex format
    int year = (dt->tm_year - 80) << 9;
    int month = (dt->tm_mon + 1) << 5;
    int day = dt->tm_mday;

    uint16_t date = year + month + day;

    uint8_t  bytes[2];

	bytes[0] = *((uint8_t*)&(date)+1);
	bytes[1] = *((uint8_t*)&(date)+0);

	// Write date
	p[i + DATE_OFFSET] = bytes[1];
	p[i + DATE_OFFSET + 1] = bytes[0];

	p[i + WRITE_DATE_OFFSET] = bytes[1];
	p[i + WRITE_DATE_OFFSET + 1] = bytes[0];

	// Form  time into appropriate hex format
	int hour = (dt->tm_hour) << 11;
    int minutes = (dt->tm_min) << 5;

    uint16_t time = hour + minutes;

    bytes[0] = *((uint8_t*)&(time)+1);
	bytes[1] = *((uint8_t*)&(time)+0);

	// Write time
	p[i + TIME_OFFSET] = bytes[1];
	p[i + TIME_OFFSET + 1] = bytes[0];

	p[i + WRITE_TIME_OFFSET] = bytes[1];
	p[i + WRITE_TIME_OFFSET + 1] = bytes[0];

	year = (access->tm_year - 80) << 9;
    month = (access->tm_mon + 1) << 5;
    day = access->tm_mday;

	date = year + month + day;

    bytes[2];

	bytes[0] = *((uint8_t*)&(date)+1);
	bytes[1] = *((uint8_t*)&(date)+0);

	p[i + ACCESS_DATE_OFFSET] = bytes[1];
	p[i + ACCESS_DATE_OFFSET + 1] = bytes[0];

	double amnt = (double)sb.st_size/512;

	// Write FAT entries
	int fat_start = findFreeEntries(p, (int)ceil(amnt));
	putFatEntries(p, fat_start, (int)ceil(amnt));

	// Write data
	int sector = (fat_start + 31);
	writeData(p, file_name, sector * 512, sb.st_size);

	uint16_t first_logical_cluster = fat_start;

	bytes[0] = *((uint8_t*)&(first_logical_cluster)+1);
	bytes[1] = *((uint8_t*)&(first_logical_cluster)+0);

	// Write first logical cluster
	p[i + FIRST_LOGICAL_CLUSTER_OFFSET] = bytes[1];
	p[i + FIRST_LOGICAL_CLUSTER_OFFSET + 1] = bytes[0];

	uint32_t size = sb.st_size;

	uint8_t four_bytes[4];

	four_bytes[0] = *((uint8_t*)&(size)+3);
	four_bytes[1] = *((uint8_t*)&(size)+2);
	four_bytes[2] = *((uint8_t*)&(size)+1);
	four_bytes[3] = *((uint8_t*)&(size));

	// Write file size
	p[i + FILE_SIZE_OFFSET] = four_bytes[3];
	p[i + FILE_SIZE_OFFSET + 1] = four_bytes[2];
	p[i + FILE_SIZE_OFFSET + 2] = four_bytes[1];
	p[i + FILE_SIZE_OFFSET + 3] = four_bytes[0];

	return;
}
