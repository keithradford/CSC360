#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

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

    char *input = NULL ;
    char *prompt = "PMan: > ";

    input = readline(prompt);

    printf("%s\n", input); //for testing purposes

    const char *split = " ";
    char *token;
    char *pid;
    char **argv = malloc(10 * sizeof(char*));
	char *arg;

    token = strtok(input, split);
    // pid = strtok(NULL, split);
    arg = strtok(NULL, split);

    int i = 0;
	while(arg != NULL){
		// if(i == 10){
		// 	expand_array(argv);
		// }
		argv[i] = malloc(strlen(arg) * sizeof(char));
		strcpy(argv[i], arg);
		free(argv[i]);
		arg = strtok(NULL, split);
		i++;
	}
	free(argv);

    printf("first: %s\nsecond: %s\n", token, pid);

	size_t n = sizeof(argv)/sizeof(argv[0]);

	for(int i = 0; i < n; i++){
		if(argv[i]){
			printf("%s\n", argv[i]);
		}
	}


    return 1;
}
