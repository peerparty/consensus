CC=gcc
CFLAGS=-I. 

all: citizen.o citcomm.o
	$(CC) -o citizen citizen.o citcomm.o -lpthread

clean:
	rm -rf *.o citizen	

