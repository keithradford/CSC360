#ifndef LINKED_LIST
#define LINKED_LIST

struct Node{
	pid_t data;
	char* directory;
	struct Node* next;
};

// Node methods
void add(struct Node** head, pid_t data, char* directory);
void print_list(struct Node* n);

#endif