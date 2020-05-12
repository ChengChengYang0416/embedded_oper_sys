#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc , char *argv[]){

  int fd;
  lcd_write_info_t display;

  if ((fd = open("/dev/lcd", O_RDWR)) < 0){
    printf("Open /dev/lcd failed.\n");
    exit(-1);
  }

  ioctl(fd, LCD_IOCTL_CLEAR, NULL);

  display.Count = sprintf((char *) display.Msg, "Hello world\n");
  ioctl(fd, LCD_IOCTL_WRITE, &display);
  sleep(3);

  ioctl(fd, LCD_IOCTL_CUR_GET, &display);
  printf("The cursor position is at (%d, %d).\n", display.CursorX, display.CursorY);

  display.Count = sprintf((char *) display.Msg, "1+4=5\n");
  ioctl(fd, LCD_IOCTL_WRITE, &display);
  sleep(3);

  int ans = 123;
  display.Count = sprintf((char *) display.Msg, "%d\n", ans);
  ioctl(fd, LCD_IOCTL_WRITE, &display);
  sleep(3);

  ioctl(fd, LCD_IOCTL_CUR_GET, &display);
  printf("The cursor position is at (%d, %d).\n", display.CursorX, display.CursorY);

  close(fd);

  return 0;
}
