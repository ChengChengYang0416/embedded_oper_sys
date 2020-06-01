CC:=g++
OBJS = sockop
TARGETS=client server

all: $(TARGETS)

$(OBJS).o: $(OBJS).c
	$(CC) -c -o $@ $<

client: client.cpp $(OBJS).o
	$(CC) -pthread -g -o client $^

server: server.cpp $(OBJS).o
	$(CC) -pthread -g -o server $^

clean:
	rm *.o
	rm $(TARGETS)
