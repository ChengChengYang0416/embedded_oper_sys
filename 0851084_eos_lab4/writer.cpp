#include <cstdio>       // fprintf(), perror()
#include <cstdlib>      // exit()
#include <cstring>      // memset()
#include <csignal>      // signal()
#include <fcntl.h>      // open()
#include <unistd.h>     // read(), write(), close()

#include <sys/socket.h> // socket(), connect()
#include <netinet/in.h> // struct sockaddr_in
#include <arpa/inet.h>  // htons()


int connfd, fd;

int main(int argc, char *argv[]){


  if (argc != 2){
    fprintf(stderr, "Usage: ./writer <sec>");
    exit(EXIT_FAILURE);
  }

  if((fd = open("/dev/mydev", O_RDWR)) < 0) {
      perror("/dev/mydev");
      exit(EXIT_FAILURE);
  }

  int i = 0;
  char wbuf[11] = "9876543210";
  for(i = 0; i <=9 ; i++){
    write(fd, &wbuf[i],1);
    sleep(1);
  }

  return 0;
}
