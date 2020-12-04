#include "utils.h"

// Function signature
void writeToFile(char *p, int start, const char *file);

int main(int argc, char *argv[]){
	int fd;
	struct stat sb;

	if(argc != 3){
		printf("Error: diskget usage must be \n./diskget <DISK> <FILE>\n");
		exit(1);
	}


	char *disk = malloc ( strlen(argv[1]) * sizeof(char) );
	char *file = malloc ( strlen(argv[2]) * sizeof(char) );
	strcpy(disk, argv[1]);	
	strcpy(file, argv[2]);

	// Open the given disk and map into p
	fd = open(disk, O_RDWR);
	fstat(fd, &sb);

	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}

	// Iterate through disk looking for file
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
		const char *file_name = getFileName(p, i, 0);
		if(strcmp(file_name, file) == 0){
			writeToFile(p, i, file_name);
			found = 1;
			break;
		}
	}

	if(!found){
		printf("File not found.\n");
		exit(1);
	}

	munmap(p, sb.st_size);
	close(fd);
	return 1;
}

/*
 * Function: writeToFile
 * Parameter(s):
 * p, a string of containing all disk information.
 * start, an integer represting the offset of the file in p
 * file, the name of the file to get
 *
 * Given the starting point parameter, a new file with the given name is created
 * and is written to character by character until the file size is hit.
 * File is written to the directory containing the executable.
 */
void writeToFile(char *p, int start, const char *file){
	int high = (p[start + FIRST_LOGICAL_CLUSTER_OFFSET + 1] & 0xFF) << 8;
	int low = p[start + FIRST_LOGICAL_CLUSTER_OFFSET] & 0xFF;
	int first_logical_cluster = high + low;

	int file_size = getFileSize(p, start);

	FILE *fp;
	fp = fopen(file, "w");
	int sector = (33 + first_logical_cluster - 2) * 512;
	for(int i = 0; i < file_size; i++){
		fputc(p[sector + i], fp);
	}
	fclose(fp);
}
