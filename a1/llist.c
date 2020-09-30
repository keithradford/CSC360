#include <stdio.h>
#include <stdlib.h>

#include "llist.h"

/*
Linked List implementation from GeesForGeeks.org
*/

struct Node;

void add(struct Node** head, pid_t data, char* directory){
	// allocate new Node
	struct Node* n = (struct Node*)malloc(sizeof(struct Node));
	n->data = data;
	n->directory = directory;
	n->next = NULL;

	// add Node to list
	struct Node* tmp = *head;
	if(*head == NULL){
		*head = n;
		printf("hello\n");
	}
	else{
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = n;
	}
	return;
}

void print_list(struct Node* n){
	int i = 0;
	while(n != NULL){
		printf("%d: %s\n", n->data, n->directory);
		n = n->next;
		i++;
	}
	printf("Total background jobs: %d\n", i);
}