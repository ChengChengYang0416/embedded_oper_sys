#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc , char *argv[]){

  char key;
  char output;
  int fd, ret;

  if ((fd = open("/dev/lcd", O_RDWR)) < 0){
    printf("Open /dev/lcd failed.\n");
    exit(-1);
  }

  ioctl(fd, KEY_IOCTL_CLEAR, key);

  while(1){

    ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
    if (ret == 0){

      switch(key){

        case 'A':
          output = '+';
          break;

        case 'B':
          output = '-';
          break;

        case 'C':
          output = '*';
          break;

        case 'D':
          output = '/';
          break;

        case '*':
          output = 'C';
          break;

        case '#':
          output = '=';
          break;

        default:
          output = key;
          break;
      }
      printf("I got %c\n", output);

    }

    if((key & 0xff) == '#') break;
  }

  close(fd);

  return 0;
}
