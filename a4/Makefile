
CC = gcc

LDLIBS = -lpthread -lrt
GLDLIBS = 
LDFLAGS  = $(LDLIBS) $(GLDLIBS) -lm

CFLAGS = -g -Wall

default: ants

ants: anthills.o aardvarks.o ring_buffer.o
	$(CC) $(CFLAGS) $(LDFLAGS) anthills.o aardvarks.o ring_buffer.o -o $@

aardvarks.o: aardvarks.c ring_buffer.o
	$(CC) $(CFLAGS) -c $< -o $@

ring_buffer.o: ring_buffer.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:	
	$(RM) ants aardvarks.o ring_buffer.o