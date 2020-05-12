CXX=g++
CFLAG = -g -Wall -std=c++11

all: server client

%.o: %.cpp
	$(CXX) -c $< $(CFLAG)

server: server.o
	$(CXX) -o $@ $^ $(CFLAG) -pthread

client: client.o
	$(CXX) -o $@ $^ $(CFLAG)

clean:
	rm -f server server.o client client.o

.PHONY: all clean
