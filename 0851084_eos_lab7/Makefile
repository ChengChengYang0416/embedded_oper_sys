CXX = g++
CFLAG = -g -Wall

all: shm_server timer shm_client sig_catch sig_send guess game

%.o: %.cpp
	$(CXX) -c $< $(CFLAG)

game: game.o
	$(CXX) -o $@ $^

guess: guess.o
	$(CXX) -o $@ $^

timer: timer.o
	$(CXX) -o $@ $^

shm_server: shm_server.o
	$(CXX) -o $@ $^

shm_client: shm_client.o
	$(CXX) -o $@ $^

sig_catch: sig_catch.o
	$(CXX) -o $@ $^

sig_send: sig_send.o
	$(CXX) -o $@ $^

clean:
	rm -f game game.o guess guess.o timer timer.o shm_server shm_server.o shm_client shm_client.o sig_catch sig_catch.o sig_send sig_send.o

.PHONY: all clean
