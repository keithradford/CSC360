#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "llist.h"

// void expand_array(char *arr[]){
// 	size_t n = sizeof(arr)/sizeof(arr[0]);
// 	char tmp[n*2][40];
// 	for(int i = 0; i < n; i++){
// 		strcpy(tmp[i], arr[i]);
// 	}
// 	tmp[n*2] = '\0';
// 	*arr = tmp;
// } 


int main(){
//------------------------LINKED LIST--------------------------
	int i = 3;

	if(fork()){
		i++;
		fork();
		i+=3;
	}
	else{
		i+=4;
		fork();
		i+=5;
	}
	printf("%d\n", i);

//--------------- PMAN----------------
 //    char *input = NULL ;
 //    char *prompt = "PMan: > ";

 //    input = readline(prompt);

 //    printf("%s\n", input); //for testing purposes

 //    const char *split = " ";
 //    char *token;
 //    char *pid;
 //    char *argv[10];
	// char *arg;

 //    token = strtok(input, split);
 //    // pid = strtok(NULL, split);
 //    arg = strtok(NULL, split);

 //    int i = 0;
	// while(arg != NULL){
	// 	// if(i == 10){
	// 	// 	expand_array(argv);
	// 	// }
	// 	printf("%s\n%ld\n", arg, strlen(arg));
	// 	strcpy(argv[i], arg);
	// 	arg = strtok(NULL, split);
	// 	i++;
	// }

 //    printf("first: %s\nsecond: %s\n", token, pid);

	// size_t n = sizeof(argv)/sizeof(argv[0]);

	// for(int i = 0; i < n; i++){
	// 	if(argv[i]){
	// 		printf("%s\n", argv[i]);
	// 	}
	// }

	// free(argv);


    return 1;
}
