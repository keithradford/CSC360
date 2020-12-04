# CSC 360 Assignment 3
These files implement utilities that perform operations on a simple file system, FAT12. Specifically, diskinfo which prints info regarding a disk, disklist which lists all files and directories
in a disk, diskget which fetches files from disks into a local directory, and diskput which puts given files from the working directory onto the disk.

## Compiling Instructions
1. Navigate to the directory for the program
2. Type "make all" in the command line to compile all 4 files
3. Type one of the following to run:
* ./diskinfo DISK
* ./disklist DISK
* ./diskget DISK FILE_NAME
* ./diskput DISK PATH_ON_DISK/FILE_NAME

## Notes
Diskput is currently broken. Seems to be a bug for bigger files, works with smaller text files. Also is putting in the wrong date. Everything else should work.
