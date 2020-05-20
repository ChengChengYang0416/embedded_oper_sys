#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


int main(int argc, char *argv[]){

  int pid = atoi(argv[1]);
  kill(pid, SIGUSR1);

  return 0;
}
