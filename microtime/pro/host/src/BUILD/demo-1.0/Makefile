#
# demo

#CC=arm-unknown-linux-gnu-gcc
CFLAG=-O0 -gdwarf-2 

demo: demo.o 
	$(CC) $(CFLAG) -o demo demo.c
#	$(STRIP) demo

clean:
	rm -f *.o demo

