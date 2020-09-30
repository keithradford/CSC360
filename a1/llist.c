#include <stdio.h>

#include "llist.h"

struct Node;

void print_list(struct Node* n){
	while(n != NULL){
		printf("%d\n", n->data);
		n = n->next;
	}
}