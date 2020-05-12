#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc , char *argv[]){

  _7seg_info_t data;

  _7seg_info_t data_5;
  _7seg_info_t data_6;
  _7seg_info_t data_7;
  _7seg_info_t data_8;

  int fd, ret, i;
  if ((fd = open("/dev/lcd", O_RDWR)) < 0){
    printf("Open /dev/lcd failed.\n");
    exit(-1);
  }

  ioctl(fd, _7SEG_IOCTL_ON, NULL);

  data_5.Mode = _7SEG_MODE_PATTERN;
  data_5.Which = _7SEG_D5_INDEX;
  data_5.Value = 0x40;
  ioctl(fd, _7SEG_IOCTL_SET, &data_5);

  data_6.Mode = _7SEG_MODE_HEX_VALUE;
  data_6.Which = _7SEG_D6_INDEX;
  data_6.Value = 3;
  ioctl(fd, _7SEG_IOCTL_SET, &data_6);

  data_7.Mode = _7SEG_MODE_HEX_VALUE;
  data_7.Which = _7SEG_D7_INDEX;
  data_7.Value = 2;
  ioctl(fd, _7SEG_IOCTL_SET, &data_7);

  data_8.Mode = _7SEG_MODE_HEX_VALUE;
  data_8.Which = _7SEG_D8_INDEX;
  data_8.Value = 1;
  ioctl(fd, _7SEG_IOCTL_SET, &data_8);
  printf("print -321 !\n");
  sleep(3);

  int ans = 200;
  char hex[10];
  sprintf(hex, "%d", ans);
  int num = (int)strtol(hex, NULL, 16);

  data.Mode = _7SEG_MODE_HEX_VALUE;
  data.Which = _7SEG_ALL;
  data.Value = num;
  ioctl(fd, _7SEG_IOCTL_SET, &data);
  printf("print 200 !\n");
  sleep(3);

  data.Value = 0x1234;
  ioctl(fd, _7SEG_IOCTL_SET, &data);
  printf("print 1234 !\n");
  sleep(3);

  data.Mode = _7SEG_MODE_PATTERN;
  data.Which = _7SEG_D5_INDEX | _7SEG_D8_INDEX;
  data.Value = 0x6d7f;
  ioctl(fd, _7SEG_IOCTL_SET, &data);
  sleep(3);

  ioctl(fd, _7SEG_IOCTL_OFF, NULL);

  close(fd);

  return 0;
}
