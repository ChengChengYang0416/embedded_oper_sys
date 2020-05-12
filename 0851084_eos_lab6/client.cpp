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

#define errexit(format,arg...) exit(printf(format,##arg))

// client of socket
int connectsock(const char *host, const char *service, const char *transport){

  struct hostent *phe;
  struct servent *pse;
  struct sockaddr_in sin;
  int s, type;

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  if ((pse = getservbyname(service, transport))){
    sin.sin_port = pse->s_port;
  }
  else if ((sin.sin_port = htons((unsigned short)atoi(service))) == 0){
    errexit("Can't get \"%s\" service entry\n", service);
  }

  if ((phe = gethostbyname(host))){
    memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
  }
  else if((sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE){
    errexit("Can't get \"%s\" host entry\n", host);
  }

  if (strcmp(transport, "udp") == 0){
    type = SOCK_DGRAM;
  }
  else {
    type = SOCK_STREAM;
  }

  s = socket(PF_INET, type, 0);
  if (s < 0){
    errexit("Can't create socket : %s\n", strerror(errno));
  }

  if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0){
    errexit("Can't connect to %s.%s : %s\n", host, service, strerror(errno));
  }
  return s;
}

int main(int argc, char *argv[]){

  if (argc != 6){
    fprintf(stderr, "Usage : %s <ip> <port> <deposit or withdraw> <amount> <time>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // create socket and connect to server
  int sockfd = connectsock(argv[1], argv[2], "tcp");

  // write message to server
  int times = atoi(argv[5]);
  char action_amount[16];
  sprintf(action_amount, "%s %s\n", argv[3], argv[4]);
  int i = 0;
  for (i = 0; i < times; ++i){
    write(sockfd, action_amount, strlen(action_amount));
  }

  close(sockfd);

  return 0;
}
