#ifndef LINKED_LIST
#define LINKED_LIST

struct Node{
	int data;
	struct Node* next;
};

void print_list(struct Node* n);

#endif