#include <stdlib.h>
#include <stdio.h>
#include "queue.h"

int main(int argc, char const *argv[])
{
	struct Queue* q = createQueue(256);
	enqueue(q, 1);
	enqueue(q, 2);
	enqueue(q, 3);
	enqueue(q, 4);
	enqueue(q, 5);
	enqueue(q, 6);
	enqueue(q, 7);
	toString(q);
	int i = dequeue(q);
	printf("%d\n", i);
	toString(q);
	i = dequeue(q);
	printf("%d\n", front(q));
	toString(q);
	dequeue(q);
	toString(q);
	dequeue(q);
	toString(q);
	dequeue(q);
	return 0;
}