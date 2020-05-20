#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

# define SHMSZ 27

int pid;
int upper_bound;
int lower_bound;

typedef struct {
  int guess;
  char result;
  int resu;
}data;
data *code_data;

// handler function when catching ctrl+c
void handler_delete(int signum) {

  shmdt(code_data);
  exit(1);

}

// timer function triggered 1 sec
void timer_handler(int signum){

  if (code_data->resu == 2){ // bingo, send signal to delete shared memory and exit
    kill(getpid(), SIGINT);
  }
  else if (code_data->resu == 3){ // bigger, set guess number bigger
    lower_bound = code_data->guess;
    code_data->guess = (code_data->guess + upper_bound)/2;
  }
  else if (code_data->resu == 1){ // smaller, set guess number smaller
    upper_bound = code_data->guess;
    code_data->guess = (code_data->guess + lower_bound)/2;
  }
  printf("[game] Guess: %d\n", code_data->guess);

  // send signal to game.cpp process
  kill(pid, SIGUSR1);
}

int main (int argc, char *argv[]){

  // handler function when catching ctrl+c
  signal(SIGINT, handler_delete);

  // receive input argument
  if (argc != 4){
    fprintf(stderr, "Usage : %s <key> <upper_bound> <pid>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  upper_bound = atoi(argv[2]);
  lower_bound = 1;
  pid = atoi(argv[3]);

  // set timer
  struct sigaction sa;
  struct itimerval timer;

  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = &timer_handler;
  sigaction(SIGVTALRM, &sa, NULL);

  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = 999999;

  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = 999999;

  setitimer(ITIMER_VIRTUAL, &timer, NULL);

  // create and attach shared memory
  int shmid;
  key_t  key;
  key = atoi(argv[1]);
  if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0){
    perror("shmget");
    exit(1);
  }
  code_data = (data *) shmat(shmid, NULL, 0);
  code_data->guess = atoi(argv[2])/2;
  code_data->resu = 4;

  // busy work
  while(1);

  return 0;
}
