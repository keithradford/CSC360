#include "utils.h"

char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sept", "Oct", "Nov", "Dec"};

int diskSize(char *p){
	int high = p[SECTOR_OFFSET + 1] << 8;
	int low = p[SECTOR_OFFSET];
	int sector_count = high + low;

	return sector_count * 512;
}

int getFileSize(char *p, int start){
	int fourth = (p[start + FILE_SIZE_OFFSET + 3] & 0xFF) << 24;
	int third = (p[start + FILE_SIZE_OFFSET + 2] & 0xFF) << 16;
	int second = (p[start + FILE_SIZE_OFFSET + 1] & 0xFF) << 8;
	int first = p[start + FILE_SIZE_OFFSET] & 0xFF;
	int file_size = first + third + second + fourth;

	return file_size;
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

	if(directory)
		return name;

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