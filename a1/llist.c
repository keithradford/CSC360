#include <stdio.h>
#include <stdlib.h>

#include "llist.h"

/*
Linked List implementation from GeesForGeeks.org
*/

struct Node;

void add(struct Node** head, pid_t pid, char* directory){
	// allocate new Node
	struct Node* n = (struct Node*)malloc(sizeof(struct Node));
	n->pid = pid;
	n->directory = directory;
	n->next = NULL;

	// add Node to list
	struct Node* tmp = *head;
	if(*head == NULL){
		*head = n;
	}
	else{
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = n;
	}
	free(n);
	return;
}

void delete_node(struct Node** head, pid_t pid){
	struct Node* tmp = *head;
	struct Node* prev = NULL;

	if(tmp != NULL && tmp->pid == pid){
		*head = tmp->next;
		free(tmp);
		return;
	}

	while(tmp != NULL && tmp->pid != pid){
		prev = tmp;
		tmp = tmp->next;
	}

	if(tmp == NULL){
		printf("Could not delete node with pid %d\nNot found in linked list.", pid);
	}

	prev->next = tmp->next;

	free(tmp);
}

void print_list(struct Node* n){
	int i = 0;
	while(n != NULL){
		printf("%d: %s\n", n->pid, n->directory);
		n = n->next;
		i++;
	}
	printf("Total background jobs: %d\n", i);
}