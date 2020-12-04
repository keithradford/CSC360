#include "queue.h"
#include "utils.h"

// Function signature
void printFiles(char *p, int start, int root, char *parent);

// Queue to hold directories in order found
struct Queue* directories = NULL;

int main(int argc, char *argv[]){
	int fd;
	struct stat sb;

	directories = createQueue(256);

	if(argc != 2){
		printf("Error: disklist usage must be \n./disklist <DISK>\n");
		exit(1);
	}

	char *disk = malloc (strlen(argv[1]) * sizeof(char));

	// Open the given disk and map into p
	strcpy(disk, argv[1]);	

	fd = open(disk, O_RDWR);
	fstat(fd, &sb);

	char * p = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (p == MAP_FAILED) {
		printf("Error: failed to map memory\n");
		exit(1);
	}

	printFiles(p, ROOT_DIRECTORY, 0, "");

	return 1;
}

/*
 * Function: printFileDate
 * Parameter(s):
 * p, a string of containing all disk information.
 * start, an integer represting the offset of the file in p
 *
 * Goes through the directory entry for the given file and prints it's date and time information.
 * Printed formating is: MMM DD YYYY HH:MM (i.e, Nov 22 2020 10:32)
 */
void printFileDate(char *p, int start){
	int time, time_low, time_high, date, date_low, date_high;
	int hours, minutes, day, month, year;
	
	time_low = p[start + TIME_OFFSET];
	time_high = p[start + TIME_OFFSET + 1] << 8;
	time = time_low + time_high;

	date_low = p[start + DATE_OFFSET];
	date_high = p[start + DATE_OFFSET + 1] << 8;
	date = date_low + date_high;
	
	// The year is stored as a value starting at 1980
	// The year is stored in the high seven bits
	year = ((date & 0xFE00) >> 9) + 1980;
	// The month is stored in the middle four bits
	month = (date & 0x1E0) >> 5;
	// The day is stored in the low five bits
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

/*
 * Function: getPath
 * Parameter(s):
 * name, a string containing the name who's path to return.
 * parent, the path of the parent directories of the file
 * Returns: A string containing a path
 *
 * Merges the two parameters into the form <parent>/<name>
 * i.e. parent is /SUB1/SUB2 and name is INPUT.TXT
 * outputs "/SUB1/SUB2/INPUT.TXT"
 */
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

	return to_ret;
}

/*
 * Function: printFiles
 * Parameter(s):
 * p, a string of containing all disk information.
 * start, an integer containing the offset of the directory to count files in
 * directory, a flag telling the program if it is in:
 * root directory (directory = 0), one level down (directory = 1), 2+ levels down (directory = 2)
 * parent, a string containg the parent directory of the file (i.e. /SUB1 is /SUB1/hello.txt's parent)
 *
 * Prints all relevent data regarding files in a disk in the given format:
 * <Directory/File> <size> <file name> <file date>
 * A directory is given by it's name, followed by a line of '=' then it's containing files are listed.
 * Directories are parsed recursively in order to print information for every file.
 * When directories are encountered, they are enqueued into a queue, the once all files in the curr
 * directory are prited, the directories are dequeued and printFiles is called for each.
 */
void printFiles(char *p, int start, int directory, char *parent){
	int dir_count = 0;
	int bound = 0;
	int sector_number = 0;
	int size = 0;

	// These if statements deal with "." and ".." in the directory entries
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

		// If directory
		if(CHECK_BIT(p[i + ATTRIBUTE_OFFSET], 4)){
			size = 0;
			const char *name = getFileName(p, i, 1);
			printf("D %10d %15s ", size, name);
			printFileDate(p, i);
			printf("\n");
			int sector = (33 + first_logical_cluster - 2) * 512;

			enqueue(directories, parent, name, sector);
		}
		else{
			const char *name = getFileName(p, i, 0);
			size = getFileSize(p, i);
			printf("F %10d %15s ", size, name);
			printFileDate(p, i);
			printf("\n");
		}
	}

	// Go through all directories and print their info
	struct Node n;
	while(!isEmpty(directories)){
		n = dequeue(directories);
		if(directory == 0){
			printf("\n/%s\n==================\n", n.name);
			printFiles(p, n.sector + 32, 1, n.name);
		}
		else{
			char *path = getPath(n.name, n.path);
			printf("\n/%s\n==================\n", path);
			printFiles(p, n.sector + 64, 2, path);
		}
	}

	return;
}
