#ifndef LINKED_LIST
#define LINKED_LIST

struct Node{
	int data;
	struct Node* next;
};

// Node methods
void add(struct Node** head, int data);
void print_list(struct Node* n);

#endif