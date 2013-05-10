
CC = gcc

LDLIBS =
GLDLIBS = 
LDFLAGS  = $(LDLIBS) $(GLDLIBS) -lm

CFLAGS = -g -Wall

default: watch 1 2 3 4

watch: watch.o
	$(CC) $(CFLAGS) $(LDFLAGS) watch.o -o $@

watch.o: watch.c
	$(CC) $(CFLAGS) -c $< -o $@

1: 1.o
	$(CC) $(CFLAGS) $(LDFLAGS) 1.o -o $@

1.o: 1.c
	$(CC) $(CFLAGS) -c $< -o $@

2: 2.o
	$(CC) $(CFLAGS) $(LDFLAGS) 2.o -o $@

2.o: 2.c
	$(CC) $(CFLAGS) -c $< -o $@

3: 3.o
	$(CC) $(CFLAGS) $(LDFLAGS) 3.o -o $@

3.o: 3.c
	$(CC) $(CFLAGS) -c $< -o $@

4: 4.o
	$(CC) $(CFLAGS) $(LDFLAGS) 4.o -o $@

4.o: 4.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:	
	$(RM) watch 1 2 3 4 watch.o 1.o 2.o 3.o 4.o