CC=gcc
CFLAGS=-I.

all: citizen.o citcomm.o
	$(CC) -o citizen citizen.o citcomm.o

clean:
	rm -rf *.o citizen	

