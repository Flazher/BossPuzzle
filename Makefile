CFLAGS=-lncurses -std=c99
objects=bb-client.o bb-server.o output.o

all:		$(objects)
			make bb-client
			make bb-server
bb-client:	bb-client.o output.o
			gcc bb-client.o output.o -o bb-client $(CFLAGS) 
bb-server:	bb-server.o output.o
			gcc bb-server.o output.o -o bb-server $(CFLAGS)
output.o:	output.c output.h
			gcc -c output.c $(CFLAGS) 
clean:
			rm $(objects)
