.phony all:
all: PMan

PMan: main.c
	gcc main.c llist.c -fno-stack-protector -lreadline -lhistory -o PMan

.PHONY clean:
clean:
	-rm -rf *.o *.exe PMan
