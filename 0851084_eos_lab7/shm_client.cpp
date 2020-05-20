#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <string>

# define SHMSZ 27

typedef struct {
  int guess;
  char result;
}data;

int main (int argc, char *argv[]){

  data *code_data;
  int shmid;
  key_t  key;

  key = 5678;
  if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0){
    perror("shmget");
    exit(1);
  }

  code_data = (data *) shmat(shmid, NULL, 0);
  code_data->guess = 50;
  code_data->result = 'm';
  //std::string myStr = "bigger";
  //strcpy(code_data->result, myStr.c_str());
  printf("%d", code_data->guess);
  printf("%c", code_data->result);

  return 0;

}
