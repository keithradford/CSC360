#ifndef QUEUE
#define QUEUE

struct Node {
	char *name;
	int sector;
};

struct Queue {
	int front, back, size;
	unsigned capacity;
	struct Node* array;
};

// Queue methods
struct Queue* createQueue(unsigned capacity);
int isFull(struct Queue* queue);
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, const char *name, int sector);
struct Node dequeue(struct Queue* queue);
struct Node front(struct Queue* queue);
void toString(struct Queue* queue);

#endif