#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

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
	struct Node* head = NULL;
	struct Node* second = NULL;
	struct Node* third = NULL;

	// head = (struct Node*)malloc(sizeof(struct Node));
	second = (struct Node*)malloc(sizeof(struct Node));
	third = (struct Node*)malloc(sizeof(struct Node));

	// head->data = 1;
	// head->directory = "/test/test/hey";
	// head->next = second;

	// second->data = 2;
	// head->directory = "/test/test/hello.c";
	// second->next = third;

	// third->data = 3;
	// head->directory = "/test/test/sup";
	// third->next = NULL;

	add(&head, 4, "/4/..");
	add(&head, 3, "hello/world");
	add(&head, 2, "/foo");
	
	print_list(head);	
	delete_node(&head, 4);

	print_list(head);	

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
