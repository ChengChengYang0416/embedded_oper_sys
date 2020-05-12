CC=arm-unknown-linux-gnu-gcc
CFLAG=-gdwarf-2

test: ptest.o Makefile
	$(CC) $(CFLAG) -o ptest -lpthread ptest.c

clean:
	rm -f *.o ptest

