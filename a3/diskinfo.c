#include "utils.h"


// Function signatures
const char *osName(char *p);
const char *diskLabel(char *p);
int freeSize(char *p, int size);
int fileAmount(char *p, int start, int root);
int fatAmount(char *p);
int sectorsPerFat(char *p);

int main(int argc, char *argv[]){
	int fd;
	struct stat sb;

	if(argc != 2){
		printf("Error: diskinfo usage must be \n./diskinfo <DISK>\n");
		exit(1);
	}
	
	char *disk = malloc ( strlen(argv[1]) * sizeof(char) );

	// Open the given disk and map into p
	strcpy(disk, argv[1]);	
	fd = open(disk, O_RDWR);
	fstat(fd, &sb);
	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
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

	// Get free size of disk
	int free_count = freeSize(p, total_size);
	printf("Free size of the disk: %d bytes\n", free_count);

	printf("==============\n");

	// Get number of files in image
	int file_amount = fileAmount(p, ROOT_DIRECTORY, 1);
	printf("The number of files in the image: %d\n", file_amount);

	printf("==============\n");

	// Get number of fat copies
	int fat_amount = fatAmount(p);
	printf("Number of FAT copies: %d\n", fat_amount);

	// Get sectors per FAT
	int sectors_per_fat = sectorsPerFat(p);
	printf("Sectors per FAT: %d\n", sectors_per_fat);
	
	munmap(p, sb.st_size);
	close(fd);

	return 1;
}

/*
 * Function: osName
 * Parameter(s):
 * p, a string of containing all disk information.
 * Returns: A string containing the OS name on the disk
 *
 * Iterates through the Boot Sector from the specified offset of the OS name
 * copying the OS name stored on the disk into a string to be printed out.
 */
const char *osName(char *p){
	char *os = malloc( OS_SIZE * sizeof(char) );
	for(int i = 0; i < OS_SIZE; i++){
		os[i] = p[OS_OFFSET + i];
	}
	os[OS_SIZE] = '\0';

	return os;
}

/*
 * Function: diskLabel
 * Parameter(s):
 * p, a string of containing all disk information.
 * Returns: A string containing the disk label of the disk
 *
 * Iterates through the Root Directory checking each directory entries attribute.
 * If the 3rd bit of the attribute is set, than the directory entry contains the disk label.
 * Once found, it copies the characters into a string to be returned.
 */
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

/*
 * Function: freeSize
 * Parameter(s):
 * p, a string of containing all disk information.
 * size, an integer containing the size of the disk
 * Returns: An integer of the amount of free bytes on the disk
 *
 * Iterates through the FAT entries counting how many are equal to zero.
 * A FAT entry represents 512 bytes in the data section, and if it is zero
 * these 512 bytes are free space.
 * Multiplies sector count by 512 to represent amount of bytes.
 */
int freeSize(char *p, int size){
	int free_count = 0;
	int odd, even;

	// Iterates through each sector in data area
	for(int n = 33; n < (size/SECTOR_SIZE); n++){
		if(getFatEntry(p, n) == 0x00){
			free_count++;
		}
	}
	free_count *= SECTOR_SIZE;

	return free_count;
}

/*
 * Function: fileAmount
 * Parameter(s):
 * p, a string of containing all disk information.
 * start, an integer containing the offset of the directory to count files in
 * directory, a flag telling the program if it is in:
 * root directory (directory = 0), one level down (directory = 1), 2+ levels down (directory = 2)
 * Returns: An integer of the amount of files on the disk
 *
 * Iterates recursively through the file structure on the disk counting the amount of files.
 * If it finds a directory, it goes to that directories location and counts the files there,
 * then returns to it's previous location.
 */
int fileAmount(char *p, int start, int directory){
	int count = 0;
	int bound = 0;
	int sector = 0;

	// These if statements deal with "." and ".." in the directory entries
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

	// Iterate through directory
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
			int sector = (33 + first_logical_cluster - 2) * 512;
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

/*
 * Function: fatAmount
 * Parameter(s):
 * p, a string of containing all disk information.
 * Returns: An integer of the amount of FAT entries on the disk
 *
 * Checks the byte in the Boot Sector representing the amount of FAT entries.
 */
int fatAmount(char *p){
	return p[FAT_NUMBER_OFFSET];
}


/*
 * Function: sectorsPerFat
 * Parameter(s):
 * p, a string of containing all disk information.
 * Returns: An integer of the amount of sectors per FAT
 *
 * Checks the byte in the Boot Sector representing the amount of sectors per FAT entry.
 */
int sectorsPerFat(char *p){
	return p[SECTORS_PER_FAT_OFFSET];
}
