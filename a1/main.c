#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>

#include "llist.h"

#define CMD_BG "bg"
#define CMD_BGLIST "bglist"
#define CMD_BGKILL "bgkill"
#define CMD_BGSTOP "bgstop"
#define CMD_BGCONT "bgstart"
#define CMD_PSTAT "pstat"
#define ARRAY_SIZE 10

void bg_entry(char **argv);
void check_zombieProcess(void);
void bgsig_entry(pid_t pid, char* cmd);
void pstat_entry(pid_t pid);
pid_t to_int(char* str);	

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
	    printf("cmd: %s\n", cmd_type);
		for(int j = 0; j < i; j++){
			argv[j] = malloc( strlen(tmp[j]) * sizeof(char) );
			strcpy(argv[j], tmp[j]);
			// printf("%s\n", argv[j]);
		}
		argv[i] = NULL;

		// call appropriate function based on command
		if (strcmp(cmd_type, CMD_BG) == 0){
			bg_entry(argv);
		}
		else if(strcmp(cmd_type, CMD_BGLIST) == 0){
			print_list(head);
		}
		else if(strcmp(cmd_type, CMD_BGKILL) == 0|| strcmp(cmd_type, CMD_BGSTOP) == 0 || strcmp(cmd_type, CMD_BGCONT) == 0){
			printf("yo\n");
			pid_t pid = to_int(argv[0]);
			bgsig_entry(pid, cmd_type);
		}
		else if(strcmp(cmd_type, CMD_PSTAT) == 0){
			pid_t pid = to_int(argv[0]);
			printf("%d\n", pid);
			pstat_entry(pid);
		}
		// else {
		// 	usage_pman();
		// }
		check_zombieProcess();
	}
	return 0;
}

void bg_entry(char **argv){
	// printf("in bg_entry: %s\n%s\n%s\n", argv[0], argv[1], argv[2]);
	pid_t pid;
	pid = fork();
	if(pid == 0){
		if(execvp(argv[0], argv) < 0){
			perror("Error on execvp");
		}
		exit(EXIT_SUCCESS);
	}
	else if(pid > 0) {
		printf("pid: %d\n", pid);
		add(&head, pid, argv[0]);
		// print_list(head);
		// store information of the background child process in your data structures
	}
	else {
		perror("fork failed");
		exit(EXIT_FAILURE);
	}
}

void bgsig_entry(pid_t pid, char* cmd){
	printf("bgentry: %d\n", pid);
	if(strcmp(cmd, CMD_BGKILL) == 0){
		// printf("killing\n");
		int ret_val = kill(pid, SIGTERM);
	}
	else if(strcmp(cmd, CMD_BGCONT) == 0){
		// printf("killing\n");
		int ret_val = kill(pid, SIGCONT);
	}
	else if(strcmp(cmd, CMD_BGSTOP) == 0){
		// printf("killing\n");
		int ret_val = kill(pid, SIGSTOP);
	}
}

void pstat_entry(pid_t pid){
	printf("fuck C\n");
	printf("%d\n", pid);
	char destination[INT_MAX] = {0};
	snprintf(destination,"/proc/%d/stat",pid);

	printf("hey\n");

	char* comm = malloc(ARRAY_SIZE * sizeof(char*));
	char state = '0';
	unsigned long utime = 0;
	unsigned long stime = 0;
	long int rss = 0;

	printf("yo\n");

	FILE* ptr = fopen(destination, "r");
	if(ptr == NULL){
		printf("File does not exist\n");
		return;
	}

	fscanf(ptr, "%*d %s %c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu %*d %*d %*d %*d %*d %*d %*u %*u %ld", comm, &state, &utime, &stime, &rss);

	fclose(ptr);

	printf("comm: %s\nstate: %c\nutime: %lu\nstime: %lu\nrss: %ld\n", comm, state, utime, stime, rss);

}

// converts a string to it's equivalent integer
pid_t to_int(char* str){
	pid_t to_ret = 0;
	int len = strlen(str);
	for(int i = 0; i < len; i++){
		to_ret = to_ret * 10 + (str[i] - '0');
	}
	return to_ret;
}

void check_zombieProcess(void){
	int status;
	int retVal = 0;
	
	// while(1) {
	// 	usleep(1000);
	// 	if(headPnode == NULL){
	// 		return ;
	// 	}
	// 	retVal = waitpid(-1, &status, WNOHANG);
	// 	if(retVal > 0) {
	// 		//remove the background process from your data structure
	// 	}
	// 	else if(retVal == 0){
	// 		break;
	// 	}
	// 	else{
	// 		perror("waitpid failed");
	// 		exit(EXIT_FAILURE);
	// 	}
	// }
	return ;
}
