# CSC360 Assignment 1
This C program creates a simple process manager which runs in the command line. 

###Compiling Instructions
1. Navigate to the directory for the program
2. Type "make PMan" in the command line to compile
3. Type "./PMan" to run the process manager

"PMan: >" should appear in the terminal where you can enter a comand.

###Commands
* bg <command> <arguments>
	* starts running a command in the background
* bglist
	* lists all processes running in the background
* bgkill <pid>
	* terminates the process with the ID pid
* bgstop <pid>
	* pauses the process with the ID pid
* bgstart <pid>
	* starts the process with the ID pid
* pstat <pid>
	* lists stats regarding the process with the ID pid

###Notes
For all processes which end on their own and become zombie processes, they will be removed from from bglist after running a few commands.
It takes a bit for the zombie processes to be cleaned up, but they are eventually removed from the linked list.