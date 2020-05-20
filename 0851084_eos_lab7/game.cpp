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
int ans;

typedef struct {
  int guess;
  char result;
  int resu;
}data;
data *code_data;
std::string myStr;


// handler function when catching ctrl+c
void handler_delete(int signum) {

  shmdt(code_data);
  exit(1);

}

// handler function when catching signal from other process
void handler (int signo, siginfo_t *info, void *context){

  if (code_data->guess == ans){ // bingo
    code_data->resu = 2;
    printf("[game] Guess %d, bingo\n", code_data->guess);
  }
  else if(code_data->guess > ans){ // smaller
    code_data->resu = 1;
    printf("[game] Guess %d, smaller\n", code_data->guess);
  }
  else if (code_data->guess < ans){ // bigger
    code_data->resu = 3;
    printf("[game] Guess %d, bigger\n", code_data->guess);
  }

}

int main (int argc, char *argv[]){

  // handler function when catching ctrl+c
  signal(SIGINT, handler_delete);

  // receive input argument
  if (argc != 3){
    fprintf(stderr, "Usage : %s <key> <guess>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  ans = atoi(argv[2]);


  // set signal for catching signal
  struct sigaction my_action;

  memset(&my_action, 0, sizeof(struct sigaction));
  my_action.sa_flags = SA_SIGINFO;
  my_action.sa_sigaction = handler;

  sigaction(SIGUSR1, &my_action, NULL);
  printf("Game PID: %d\n", getpid());

  // create and attach shared memory
  int shmid;
  key_t  key;
  key = atoi(argv[1]);
  if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0){
    perror("shmget");
    exit(1);
  }
  code_data = (data *) shmat(shmid, NULL, 0);

  // busy work
  while(1);

  return 0;
}
