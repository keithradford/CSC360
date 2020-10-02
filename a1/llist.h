#ifndef LINKED_LIST
#define LINKED_LIST

struct Node{
	pid_t pid;
	char* directory;
	struct Node* next;
};

// Node methods
void add(struct Node** head, pid_t pid, char* directory);
void delete_node(struct Node** head, pid_t pid);
void print_list(struct Node* n);
bool is_empty(struct Node** head);

#endif