#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "queue.h"

struct Queue;

struct Node;

struct Queue* createQueue(unsigned capacity){
	struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
	queue->capacity = capacity;
	queue->front = queue->size = 0;

	queue->back = capacity -1;
	struct Node arr[capacity];
	queue->array = arr;

	return queue;
}

int isFull(struct Queue* queue){
	return (queue->size == queue->capacity);
}

int isEmpty(struct Queue* queue){
	return (queue->size == 0);
}

void enqueue(struct Queue* queue, char *path, const char *name, int sector){
	// printf("enqueue %s %d\n", name, sector);
	struct Node n;
	n.name = name;
	n.path = path;
	n.sector = sector;

	if(isFull(queue))
		return;
	queue->back = (queue->back + 1) % queue->capacity;
	queue->array[queue->back] = n;
	queue->size = queue->size + 1;
	// printf("Enqueued %d\n", id);
}

struct Node dequeue(struct Queue* queue){
	if(isEmpty(queue))
		printf("empty\n");
	struct Node n = queue->array[queue->front];
	queue->front = (queue->front + 1) % queue->capacity;
	queue->size = queue->size - 1;
	return n;
}

struct Node front(struct Queue* queue){
	if(isEmpty(queue))
		printf("empty\n");
	return queue->array[queue->front];
}

void toString(struct Queue* queue){
	int len = sizeof(queue->array);
	struct Node n;
	for(int i = 0; i < len; i++){
		n = queue->array[i];
		if(!n.name)
			break;
		printf("%s %d\n", n.name, n.sector);
	}
	// printf("\n");
}

