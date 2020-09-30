#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

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

struct Node* head = NULL;

int main(){
	// head = (struct Node*)malloc(sizeof(struct Node));

	while(1){	
		char *cmd = readline("PMan: > ");
		
		const char *split = " ";
   		char *cmd_type;
   		char *pid;
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
		// else if(cmd_type == CMD_BGLIST){
		// 	bglist_entry();
		// }
		// else if(cmd_type == CMD_BGKILL || cmd_type == CMD_BGSTOP || cmd_type == CMD_BGCONT){
		// 	pid = argv[0];
		// 	bgsig_entry(pid, cmd_type);
		// }
		// else if(cmd_type == CMD_PSTAT){
		// 	pid = argv[0];
		// 	pstat_entry(pid);
		// }
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
