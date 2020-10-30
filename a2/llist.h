#ifndef LINKED_LIST
#define LINKED_LIST

struct Node{
    int user_id;
	int class_type;
	int service_time;
	int arrival_time;
	struct Node* next;
};

// Node methods
struct Node* getNode(struct Node** head, int user_id);
void add(struct Node** head, int user_id, int class_type, int service_time, int arrival_time);
void delete_node(struct Node** head, int user_id);

#endif