.phony all:
all: PMan

PMan: main.c
	gcc main.c llistc -lreadline -lhistory -o PMan

.PHONY clean:
clean:
	-rm -rf *.o *.exe PMan
