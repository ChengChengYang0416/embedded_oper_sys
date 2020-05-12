CC=arm-unknown-linux-gnu-gcc
CFLAG=-gdwarf-2

all: client server

client: client.o Makefile
	$(CC) $(CFLAG) -o client client.c

server: server.o Makefile
	$(CC) $(CFLAG) -o server server.c

clean:
	rm -f *.o server client

