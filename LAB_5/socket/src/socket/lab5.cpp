#ifndef _SOCKOP_H_
#define _SOCKOP_H_

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define errexit(format,arg...) exit(printf(format,##arg))
#define BUFSIZE 1024

pid_t childpid;

void handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void childfunc(int fd_child){
  dup2(fd_child, 1);
  close(fd_child);
  execl("/usr/games/sl", "/usr/games/sl", "-l", NULL);
}

void parentfunc(){

}

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

int main(int argc, char *argv[]){

  signal(SIGCHLD, handler);

  int sockfd, connfd;
  struct sockaddr_in addr_cln;
  socklen_t sLen = sizeof(addr_cln);

  if (argc != 2){
    errexit("Usage : %s port\n", argv[0]);
  }



  sockfd = passivesock(argv[1], "tcp", 10);

  while (1) {

      connfd = accept(sockfd, (struct sockaddr *) &addr_cln, &sLen);
      if (connfd == -1){
        perror("accept");
        errexit("Error : accept()\n");
      }

      childpid = fork();
      if(childpid >= 0){
        if(childpid == 0){
          childfunc(connfd);
        }
        else{
          parentfunc();
        }
      }
      else{
        perror("fork");
        exit(0);
      }
      close(connfd);

  }

  close(sockfd);

  return 0;
}

#endif
