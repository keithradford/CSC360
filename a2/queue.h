#ifndef QUEUE
#define QUEUE

struct Queue {
	int front, back, size;
	unsigned capacity;
	int* array;
};

// Queue methods
struct Queue* createQueue(unsigned capacity);
int isFull(struct Queue* queue);
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, int id);
int dequeue(struct Queue* queue);

#endif