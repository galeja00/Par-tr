par-tr: par-tr.o
	gcc -o par-tr par-tr.o 

par-tr.o: par-tr.c
	gcc -c par-tr.c