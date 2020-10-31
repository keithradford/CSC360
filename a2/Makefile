.phony all:
all: ACS

ACS: main.c
	gcc main.c llist.c queue.c -lpthread -o ACS

.PHONY clean:
clean:
	-rm -rf *.o *.exe ACS
