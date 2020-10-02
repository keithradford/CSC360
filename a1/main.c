#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "llist.h"

// global variables
#define CMD_BG "bg"
#define CMD_BGLIST "bglist"
#define CMD_BGKILL "bgkill"
#define CMD_BGSTOP "bgstop"
#define CMD_BGCONT "bgstart"
#define CMD_PSTAT "pstat"
#define ARRAY_SIZE 10
#define BUFFER 256

// function declarations
void bg_entry(char **argv);
void check_zombieProcess();
void bgsig_entry(pid_t pid, char* cmd);
void pstat_entry(pid_t pid);
pid_t string_to_int(char* str);	

struct Node* head = NULL;

int main(){
	while(1){	
		char *cmd = readline("PMan: > ");
		
		const char *split = " ";
   		char *cmd_type;
   		char **tmp = malloc( ARRAY_SIZE * sizeof(char*) );
   		char *arg;
   		
   		// parse command and arguments from readline
    	cmd_type = strtok(cmd, split);
    	arg = strtok(NULL, split);

    	// put arguments into a temp array of strings
    	int i = 0;
		while(arg != NULL){
			tmp[i] = malloc( strlen(arg) * sizeof(char) );
			strcpy(tmp[i], arg);
			arg = strtok(NULL, split);
			i++;
		}

		// put arguments into appropriately sized array
		char **argv = malloc( (i + 1) * sizeof(char*) );
		for(int j = 0; j < i; j++){
			argv[j] = malloc( strlen(tmp[j]) * sizeof(char) );
			strcpy(argv[j], tmp[j]);
		}
 		argv[i] = NULL;

		// call appropriate function based on command
		// bg <command> <arguments>
		if (strcmp(cmd_type, CMD_BG) == 0){
			bg_entry(argv);
		}
		// bglist
		else if(strcmp(cmd_type, CMD_BGLIST) == 0){
			print_list(head);
		}
		else if(strcmp(cmd_type, CMD_BGKILL) == 0 	// bgkill <pid>
			|| strcmp(cmd_type, CMD_BGSTOP) == 0	// bgstop <pid>
			|| strcmp(cmd_type, CMD_BGCONT) == 0){	// bgstart <pid>
			pid_t pid = string_to_int(argv[0]);
			bgsig_entry(pid, cmd_type);
		}
		// pstat <pid>
		else if(strcmp(cmd_type, CMD_PSTAT) == 0){
			pid_t pid = string_to_int(argv[0]);
			pstat_entry(pid);
		}
		check_zombieProcess();
	}
	return 0;
}

/*
 * Function: bg_entry
 * Parameter(s):
 * argv, an array of arguments
 *
 * Runs a specified program (argv[0]) in the background as a child process.
 */
void bg_entry(char **argv){
	pid_t pid = 0;
	pid = fork();
	// child process
	if(pid == 0){
		if(execvp(argv[0], argv) < 0){
			perror("Error on execvp");
		}
		exit(EXIT_SUCCESS);
	}
	// parent process
	else if(pid > 0) {
		add(&head, pid, argv[0]);
	}
	else {
		perror("Fork failed");
		exit(EXIT_FAILURE);
	}
}

/*
 * Function: bgsig_entry
 * Parameter(s):
 * pid, a process id
 * cmd, the command to execute
 *
 * Terminates, stops, or starts the process given by pid.
 * Stopping a process allows for it to be started again.
 * Terminating a process perma stops the process.
 */
void bgsig_entry(pid_t pid, char* cmd){
	// if bgkill is passed in, kill  process <pid>
	if(strcmp(cmd, CMD_BGKILL) == 0){
		delete_node(&head, pid);
		int ret_val = kill(pid, SIGTERM);
	}
	// if bgstart is passed in, start process <pid> again
	else if(strcmp(cmd, CMD_BGCONT) == 0){
		int ret_val = kill(pid, SIGCONT);
	}
	// if bgstop is passed in, stop process <pid>
	else if(strcmp(cmd, CMD_BGSTOP) == 0){
		int ret_val = kill(pid, SIGSTOP);
	}
}

/*
 * Function: pstat_entry
 * Parameter(s):
 * pid, a process id
 *
 * Provides a detailed list of the stats and status of a process.
 * Prints the filename of the executable, the process state, time scheduled for the process in user mode,
 * time scheduled for the process in kernal mode (clock ticks), resident set size,
 * volunatry context switches, and nonvoluntary context switches in that order.
 */
void pstat_entry(pid_t pid){
	char stat[BUFFER];
	//destination of the program's stat file
	sprintf(stat,"/proc/%d/stat",pid); 

	// stat declarations
	char* comm = malloc(ARRAY_SIZE * sizeof(char*));
	char state = '0';
	unsigned long utime = 0;
	unsigned long stime = 0;
	long int rss = 0;
	long int voluntary_ctxt_switches = 0;
	long int nonvoluntary_ctxt_switches = 0;

	// open file and parse stats
	FILE* ptr = fopen(stat, "r");
	if(ptr == NULL){
		printf("File does not exist\n");
		return;
	}
	fscanf(ptr, "%*d %s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %*u %ld", comm, &state, &utime, &stime, &rss);
	fclose(ptr);

	// prints the stats
	printf("comm: %s\nstate: %c\nutime: %lu\nstime: %lu\nrss: %ld\n", comm, state, utime/sysconf(_SC_CLK_TCK), stime/sysconf(_SC_CLK_TCK), rss);

	//destination of the program's status file
	sprintf(stat,"/proc/%d/status",pid);

	// open file and parse status
	ptr = fopen(stat, "r");
	if(ptr == NULL){
		printf("File does not exist\n");
		return;
	}
	char line[BUFFER];
	while(fgets(line, BUFFER, ptr)){
		char tmp[BUFFER];
		strcpy(tmp, line);
		char *str = strtok(tmp, ":");
		if(strcmp(str, "voluntary_ctxt_switches") == 0){
			printf("%s", line);
		}
		else if(strcmp(str, "nonvoluntary_ctxt_switches") == 0){
			printf("%s\n", line);
		}
	}

	free(comm);
	fclose(ptr);
}

/*
 * Function: string_to_int
 * Parameter(s):
 * str, a string of numbers
 * Returns: an integer
 * 
 * Converts a string of numbers to an integer
 * i.e. "1234" to the int 1234
 */
pid_t string_to_int(char* str){
	pid_t to_ret = 0;
	int len = strlen(str);
	for(int i = 0; i < len; i++){
		to_ret = to_ret * 10 + (str[i] - '0');
	}
	return to_ret;
}

/*
 * Function: check_zombieProcess
 *
 * Checks for any zombie processes.
 * Removes any potential zombie processes from the linked list containing running proccesses.
 */
void check_zombieProcess(){
	int status;
	int retVal = 0;
	
	while(1) {
		usleep(1000);
		if(head == NULL){
			return ;
		}
		retVal = waitpid(-1, &status, WNOHANG);
		if(retVal > 0) {
			// remove the background process from linked list
			delete_node(&head, retVal);
		}
		else if(retVal == 0){
			break;
		}
		else{
			perror("waitpid failed");
			exit(EXIT_FAILURE);
		}
	}
	return ;
}
