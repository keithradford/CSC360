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
#define EXTENSION_SIZE 3
#define FILE_SIZE_OFFSET 28
#define FIRST_LOGICAL_CLUSTER_OFFSET 26
#define NAME_SIZE 9
#define ROOT_DIRECTORY 9728

const char *getFileName(char *p, int start);
void writeToFile(char *p, int start, const char *file);

int main(int argc, char *argv[]){
	// char *file_name = malloc( (NAME_SIZE + EXTENSION_SIZE + 1) * (sizeof(char)) );

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

	fd = open(disk, O_RDWR);
	fstat(fd, &sb);

	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); // p points to the starting pos of your mapped memory
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}

	int found = 0;
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
		const char *file_name = getFileName(p, i);
		if(strcmp(file_name, file) == 0){
			printf("Matched: %s and %s\n", file_name, file);
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
		// printf("yo %c\n", p[start + j]);
	}
	name[j] = '\0';

	int i = 0;
	for(int k = 8; k < 11; k++){
		// printf("%c\n", p[start + k]);
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

	// printf("%s\n", file_name);

	return file_name;
}

void writeToFile(char *p, int start, const char *file){
	int high = p[start + FIRST_LOGICAL_CLUSTER_OFFSET + 1] << 8;
	int low = p[start + FIRST_LOGICAL_CLUSTER_OFFSET];
	int first_logical_cluster = high + low;

	int fourth = (p[start + FILE_SIZE_OFFSET + 3] & 0xFF) << 24;
	int third = (p[start + FILE_SIZE_OFFSET + 2] & 0xFF) << 16;
	int second = (p[start + FILE_SIZE_OFFSET + 1] & 0xFF) << 8;
	int first = p[start + FILE_SIZE_OFFSET] & 0xFF;
	int file_size = first + third + second + first;

	// printf("%x %x %x %x\n", fourth, third, second, first);

	// printf("First Logical Cluster: %x\nFile Size: %d\n", first_logical_cluster, file_size);
	FILE *fp;
	fp = fopen(file, "w");
	int sector = (33 + first_logical_cluster - 2) * 512;
	for(int i = 0; i < file_size; i++){
		if((p[sector + i] & 0xFF) == 0x00)
			break;
		fputc(p[sector + i], fp);
	}
	fclose(fp);
}
