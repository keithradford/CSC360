.phony all:
all: diskinfo disklist diskget diskput

diskinfo: main.c
	gcc main.c -o diskinfo

disklist: main.c
	gcc main.c -o disklist

diskget: main.c
	gcc main.c -o diskget

diskput: main.c
	gcc main.c -o diskput

.PHONY clean:
clean:
	-rm -rf *.o *.exe ACS
