.phony all:
all: ACS

ACS: main.c
	gcc main.c llist.c queue.c -pthread -o ACS

test: test.c
	gcc test.c llist.c queue.c -pthread -o test

.PHONY clean:
clean:
	-rm -rf *.o *.exe ACS
