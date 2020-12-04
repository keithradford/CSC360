#ifndef UTILS
#define UTILS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#define CHECK_BIT(var, pos) (((var)>>(pos)) & 1)

#define ACCESS_DATE_OFFSET 18
#define ATTRIBUTE_OFFSET 11
#define DATA_OFFSET 16896
#define DATE_OFFSET 16
#define DIRECTORY_SIZE 32
#define EXTENSION_OFFSET 8
#define EXTENSION_SIZE 3
#define FAT_NUMBER_OFFSET 16
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

char *months[12];

// methods
int getFatEntry(char *p, int n);
int diskSize(char *p);
const char *getFileName(char *p, int start, int directory);
int getFileSize(char *p, int start);

#endif