.phony all:
all: diskinfo disklist diskget diskput

diskinfo: diskinfo.c
	gcc diskinfo.c -o diskinfo

disklist: disklist.c
	gcc disklist.c queue.c -o disklist

diskget: diskget.c
	gcc diskget.c -o diskget

diskput: diskput.c
	gcc diskput.c -o diskput -lm

.PHONY clean:
clean:
	-rm -rf *.o *.exe diskinfo disklist diskget diskput ANS1.PDF REMINDER.TXT ICEBERGS.TEX
