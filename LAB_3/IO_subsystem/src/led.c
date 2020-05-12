#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc , char *argv[]){

  int fd;
  int retval;
  unsigned short data;

  unsigned short data_9;
  unsigned short data_10;
  unsigned short data_11;
  unsigned short data_12;
  unsigned short data_13;
  unsigned short data_14;
  unsigned short data_15;
  unsigned short data_16;
  unsigned short data_arr[8] = {1, 2, 3, 4, 5, 6, 7, 8};

  if ((fd = open("/dev/lcd", O_RDWR)) < 0){
    printf("Open /dev/lcd failed.\n");
    exit(-1);
  }

  data = LED_ALL_ON;
  ioctl(fd, LED_IOCTL_SET, &data);
  printf("Turn of all LED lamps.\n");
  sleep(3);

  data = LED_ALL_OFF;
  ioctl(fd, LED_IOCTL_SET, &data);
  printf("Turn off all LED lamps.\n");
  sleep(3);

  data = LED_D9_INDEX;
  ioctl(fd, LED_IOCTL_BIT_SET, &data);
  printf("Turn on D9.\n");
  sleep(3);

  data = LED_D10_INDEX;
  ioctl(fd, LED_IOCTL_BIT_SET, &data);
  printf("Turn on D10.\n");
  sleep(3);

  long long int binaryNumber = 0;
  int remainder, i = 1, step = 1;
  int n = 118;
  int arr[8] = {0, 0, 0, 0, 0, 0, 0, 0};

  while (n != 0){
    remainder = n%2;
    step++;
    n /= 2;
    binaryNumber += remainder*i;
    i *= 10;
  }
  printf("binary = %d\n", binaryNumber);
  printf("\n");
  for (i = 0; i < 8; i++){
    printf("binary pa 10 = %d\n", binaryNumber%10);
    arr[7-i] = binaryNumber%10;
    binaryNumber = binaryNumber/10;
  }
  for (i = 0; i < 8; i++){
    printf("%d", arr[i]);
    data = i;
    if (arr[i] == 1){
      ioctl(fd, LED_IOCTL_BIT_SET, &data);
    }
    else{
      ioctl(fd, LED_IOCTL_BIT_CLEAR, &data);
    }
  }


  //printf("binary pa 10 = %d\n", binaryNumber%10);
  //binaryNumber = binaryNumber/10;
  //printf("binary = %d\n", binaryNumber);

/*
  for (i = 1; i < 9; i++){
    if (binaryNumber%10 == 1){
      data = data_arr[9-i];
      ioctl(fd, LED_IOCTL_BIT_SET, &data);
      binaryNumber = binaryNumber/10;
    }
    else{
      data = data_arr[9-i];
      ioctl(fd, LED_IOCTL_BIT_CLEAR, &data);
    }
  }
  sleep(3);*/
  close(fd);

  return 0;
}
