#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>


int main(){

    char *input = NULL ;
    char *prompt = "PMan: > ";

    input = readline(prompt);

    printf("%s\n", input); //for testing purposes

    const char *split = " ";
    char *token;
    char *pid;
    char *argv;

    token = strtok(input, split);
    pid = strtok(NULL, split);
    argv = strtok(NULL, split);

    printf("first: %s\nsecond: %s\nthird: %s\n", token, pid, argv);


    return 1;
}

