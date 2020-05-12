CXX = g++
CFLAG = -g -Wall -std=c++11

all: lab5

%.o: %.cpp
	$(CXX) -c $< $(CFLAG)

lab5: lab5.o
	$(CXX) -o $@ $^ $(CFLAG)

clean:
	rm -rf lab5 lab5.o

.PHONY: all clean