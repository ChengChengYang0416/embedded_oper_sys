#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/sem.h>
#include <pthread.h>

#define errexit(format,arg...) exit(printf(format,##arg))
#define BUFSIZE 1024
#define SEM_MODE 0666

int sockfd, semid, account = 0;

void handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

// server of socket
int passivesock(const char *service, const char *transport, int qlen){

  struct servent *pse;
  struct sockaddr_in sin;
  int s, type;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = INADDR_ANY;
  if ((pse = getservbyname(service, transport))){
    sin.sin_port = htons(ntohs((unsigned short)pse->s_port));
  }
  else if ((sin.sin_port = htons((unsigned short)atoi(service))) == 0){
    errexit("Can't find \"%s\" service entry\n", service);
  }

  if (strcmp(transport, "udp") == 0){
    type = SOCK_DGRAM;
  }
  else{
    type = SOCK_STREAM;
  }

  s = socket(PF_INET, SOCK_STREAM, 0);
  if (s < 0){
    errexit("Can't create socket : %s\n", strerror(errno));
  }

  int yes = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

  if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0){
    errexit("Can't listen on port %s : %s\n", service, strerror(errno));
  }

  if (type == SOCK_STREAM && listen(s, qlen) < 0){
    errexit("Can't listen on port %s : %s\n", service, strerror(errno));
  }

  return s;
}

int P(){

  struct sembuf sop;
  sop.sem_num = 0;
  sop.sem_op = -1;
  sop.sem_flg = 0;

  if (semop(semid, &sop, 1) < 0){
    fprintf(stderr, "P(): semop failed: %s\n", strerror(errno));
    return -1;
  }
  else {
    return 0;
  }
}

int V(){
  struct sembuf sop;
  sop.sem_num = 0;
  sop.sem_op = 1;
  sop.sem_flg = 0;

  if (semop(semid, &sop, 1) < 0){
    fprintf(stderr, "V(): semop failed: %s\n", strerror(errno));
    return -1;
  }
  else{
    return 0;
  }
}

void *money(void *conn) {
    int fd = *((int *)conn);
    int amount = 0;
    int i = 0;
    char buf[16] = {};
    char ch;

    // read message from client
    while (read(fd, &ch, 1)) {
        if (ch != '\n') {
          buf[i] = ch;
          i++;
        }
        else {
          // adjust account with race condition
          if (strncmp(buf, "deposit", 7) == 0) {
              amount = atoi(&buf[8]);
              P();
              account += amount;
              printf("After deposit: %d\n", account);
              V();
          }
          else if (strncmp(buf, "withdraw", 8) == 0) {
              amount = atoi(&buf[9]);
              P();
              account -= amount;
              printf("After withdraw: %d\n", account);
              V();
          }
          i = 0;
          memset(buf, 0, sizeof(buf));
        }
    }

    free(conn);
    pthread_exit(nullptr);
}

int main(int argc, char *argv[]){

  signal(SIGINT, handler);

  if (argc != 2) {
      fprintf(stderr, "Usage: %s <port>\n", argv[0]);
      exit(EXIT_FAILURE);
  }

  // create socket and bind socket to port
  sockfd = passivesock(argv[1], "tcp", 10);
  struct sockaddr_in addr_cln;
  socklen_t sLen = sizeof(addr_cln);

  // create semaphore
  int key = 428361733;
  if ((semid = semget(key, 1, IPC_CREAT)) == -1) {
      perror("Creation of semaphore failed.\n");
      exit(EXIT_FAILURE);
  }

  // set semaphore s[0] value to initial value (val)
  int val = 1;
  if (semctl(semid, 0, SETVAL, val) == -1) {
      perror("Unable to initialize semaphore.\n");
      exit(EXIT_FAILURE);
  }

  while (true) {

    // wait for connection
    int *connfd = (int *)malloc(sizeof(int));
    if ((*connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen)) == -1) {
        perror("Error return from accept()");
        exit(EXIT_FAILURE);
    }

    // create thread
    pthread_t thread;
    if (pthread_create(&thread, nullptr, money, (void *)connfd) != 0) {
        perror("Error return for pthread_create()");
        exit(-1);
    }
  }

  return 0;
}
