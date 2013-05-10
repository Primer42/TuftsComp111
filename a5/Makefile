all: a.out see.R
a.out: t5.o a5.o 
	gcc -g t5.o a5.o
t5: t5.o a5.o 
	gcc -g -o t5 t5.o a5.o
t5.o: t5.c programs.c t5.h 
	gcc -g -c t5.c 
a5.o: a5.c t5.h 
	gcc -g -c a5.c 
t5.c: /comp/111/a/a5/t5.c
	ln -s /comp/111/a/a5/t5.c .
see.R: /comp/111/a/a5/see.R
	ln -s /comp/111/a/a5/see.R .
t5.h: /comp/111/a/a5/t5.h 
	ln -s /comp/111/a/a5/t5.h .
programs.c: /comp/111/a/a5/programs.c 
	ln -s /comp/111/a/a5/programs.c .
