#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "queue.h"

struct Queue;

struct Queue* createQueue(unsigned capacity){
	struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
	queue->capacity = capacity;
	queue->front = queue->size = 0;

	queue->back = capacity -1;
	queue->array = (int*)malloc(queue->capacity * sizeof(int));

	return queue;
}

int isFull(struct Queue* queue){
	return (queue->size == queue->capacity);
}

int isEmpty(struct Queue* queue){
	return (queue->size == 0);
}

void enqueue(struct Queue* queue, int id){
	if(isFull(queue))
		return;
	queue->back = (queue->back + 1) % queue->capacity;
	queue->array[queue->back] = id;
	queue->size = queue->size + 1;
	// printf("Enqueued %d\n", id);
}

int dequeue(struct Queue* queue){
	if(isEmpty(queue))
		return INT_MIN;
	int id = queue->array[queue->front];
	queue->front = (queue->front + 1) % queue->capacity;
	queue->size = queue->size - 1;
	return id;
}

int front(struct Queue* queue){
	if(isEmpty(queue))
		return INT_MIN;
	return queue->array[queue->front];
}

void toString(struct Queue* queue){
	int len = sizeof(queue->array);
	for(int i = 0; i < len; i++){
		printf("%d ", queue->array[i]);
	}
	printf("\n");
}

