#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "llist.h"

/*
Linked List implementation from GeesForGeeks.org
*/

struct Node;

void add(struct Node** head, int user_id, int class_type, int service_time, int arrival_time){
	// allocate new Node
	struct Node* n = (struct Node*)malloc(sizeof(struct Node));
	n->user_id = user_id;
	n->class_type = class_type;
	n->service_time = service_time;
	n->arrival_time = arrival_time;
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
	return;
}

void delete_node(struct Node** head, int user_id){
	struct Node* tmp = *head;
	struct Node* prev = NULL;

	if(tmp != NULL && tmp->user_id == user_id){
		*head = tmp->next;
		free(tmp);
		return;
	}

	while(tmp != NULL && tmp->user_id != user_id){
		prev = tmp;
		tmp = tmp->next;
	}

	if(tmp == NULL){
		return;
	}

	prev->next = tmp->next;

	free(tmp);
}
