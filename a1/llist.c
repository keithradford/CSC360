#include <stdio.h>
#include <stdlib.h>

#include "llist.h"

struct Node;

void add(struct Node** head, int data){
	// alocate new Node
	struct Node* n = (struct Node*)malloc(sizeof(struct Node));
	n->data = data;
	n->next = NULL;

	// add Node to list
	struct Node* tmp = *head;
	if(*head == NULL){
		*head = tmp;
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
	while(n != NULL){
		printf("%d\n", n->data);
		n = n->next;
	}
}