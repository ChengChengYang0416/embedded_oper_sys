CC:=g++
OBJS = sockop
TARGETS = checker hw3
TAGS = -pthread

all: $(TARGETS)	

checker: checker.cpp $(OBJS).c
	$(CC) -o $@ $^

hw3: hw3.cpp $(OBJS).o
	$(CC) -pthread -g -o hw3 $^

clean:
	rm $(TARGETS)
