#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>


int main(){

    char *input = NULL ;
    char *prompt = "PMan:  >";

    input = readline(prompt);

    printf("%s\n", input); //for testing purposes

    if(strcmp(input, "bg"))

    return 1;
}

