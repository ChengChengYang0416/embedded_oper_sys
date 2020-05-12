#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdbool.h>

// declare for keypad
char key;
int fd, ret;
int ans;

// declare for LCD
lcd_write_info_t display;

// declare for 7 segment
_7seg_info_t data;
_7seg_info_t data_neg_sgn;
_7seg_info_t data_neg_5;
_7seg_info_t data_neg_6;
_7seg_info_t data_neg_7;
_7seg_info_t data_neg_8;

// declare for led
unsigned short led;

struct Number
{
  int num;
  char oper;
};

char change_operator(char keyin){

  char oper;
  switch(keyin){
    case 'A':
      oper = '+';
      break;

    case 'B':
      oper = '-';
      break;

    case 'C':
      oper = '*';
      break;

    case 'D':
      oper = '/';
      break;

    case '*':
      oper = 'C';
      break;

    case '#':
      oper = '=';
      break;

    default:
      oper = keyin;
      break;
  }
  return oper;

}

int  calculator(char ch[], int index){

  int answer = 0;
  int i_cal = 0;

  struct Number first, second, third;
  first.num = 0;
  first.oper = '+';

  while(first.oper != '='){
    second.num = ch[i_cal] - '0';
    i_cal++;
    second.oper = ch[i_cal];
    i_cal++;
    while (second.oper == '*' || second.oper == '/'){
      third.num = ch[i_cal] - '0';
      i_cal++;
      third.oper = ch[i_cal];
      i_cal++;
      if (second.oper == '*'){
        second.num *= third.num;
      }
      else if (second.oper == '/'){
        second.num /= third.num;
      }
      second.oper = third.oper;
    }
    if (first.oper == '+'){
      first.num += second.num;
    }
    else if (first.oper == '-'){
      first.num -= second.num;
    }
    first.oper = second.oper;
  }

  answer = first.num;

  printf("%d\n", answer);
  return answer;
}

void print_positive(int n){
  char hex[10];
  sprintf(hex, "%d", n);
  int num = (int)strtol(hex, NULL, 16);
  data.Mode = _7SEG_MODE_HEX_VALUE;
  data.Which = _7SEG_ALL;
  data.Value = num;
  ioctl(fd, _7SEG_IOCTL_SET, &data);
}

void print_negative(int n){
  int n_abs = abs(n);
  int digits = 0;
  while(n_abs != 0){
    n_abs/=10;
    digits++;
  }

  switch(digits){
    case 1:
      data_neg_5.Mode = _7SEG_MODE_PATTERN;
      data_neg_5.Which = _7SEG_D5_INDEX;
      data_neg_5.Value = 0;
      ioctl(fd, _7SEG_IOCTL_SET, &data_neg_5);

      data_neg_6.Mode = _7SEG_MODE_PATTERN;
      data_neg_6.Which = _7SEG_D6_INDEX;
      data_neg_6.Value = 0;
      ioctl(fd, _7SEG_IOCTL_SET, &data_neg_6);

      data_neg_8.Mode = _7SEG_MODE_HEX_VALUE;
      data_neg_8.Which = _7SEG_D8_INDEX;
      data_neg_8.Value = abs(n);
      ioctl(fd, _7SEG_IOCTL_SET, &data_neg_8);

      data_neg_sgn.Mode = _7SEG_MODE_PATTERN;
      data_neg_sgn.Which = _7SEG_D7_INDEX;
      data_neg_sgn.Value = 0x40;
      ioctl(fd, _7SEG_IOCTL_SET, &data_neg_sgn);
      break;

    case 2:
      data_neg_5.Mode = _7SEG_MODE_PATTERN;
      data_neg_5.Which = _7SEG_D5_INDEX;
      data_neg_5.Value = 0;
      ioctl(fd, _7SEG_IOCTL_SET, &data_neg_5);

      data_neg_7.Mode = _7SEG_MODE_HEX_VALUE;
      data_neg_7.Which = _7SEG_D7_INDEX;
      data_neg_7.Value = abs(n)/10;
      ioctl(fd, _7SEG_IOCTL_SET, &data_neg_7);

      data_neg_8.Mode = _7SEG_MODE_HEX_VALUE;
      data_neg_8.Which = _7SEG_D8_INDEX;
      data_neg_8.Value = abs(n)%10;
      ioctl(fd, _7SEG_IOCTL_SET, &data_neg_8);

      data_neg_sgn.Mode = _7SEG_MODE_PATTERN;
      data_neg_sgn.Which = _7SEG_D6_INDEX;
      data_neg_sgn.Value = 0x40;
      ioctl(fd, _7SEG_IOCTL_SET, &data_neg_sgn);
      break;

    case 3:
      data_neg_6.Mode = _7SEG_MODE_HEX_VALUE;
      data_neg_6.Which = _7SEG_D6_INDEX;
      data_neg_6.Value = abs(n)/100;
      ioctl(fd, _7SEG_IOCTL_SET, &data_neg_6);

      data_neg_7.Mode = _7SEG_MODE_HEX_VALUE;
      data_neg_7.Which = _7SEG_D7_INDEX;
      data_neg_7.Value = (abs(n)/10)%10;
      ioctl(fd, _7SEG_IOCTL_SET, &data_neg_7);

      data_neg_8.Mode = _7SEG_MODE_HEX_VALUE;
      data_neg_8.Which = _7SEG_D8_INDEX;
      data_neg_8.Value = (abs(n)%100)%10;
      ioctl(fd, _7SEG_IOCTL_SET, &data_neg_8);

      data_neg_sgn.Mode = _7SEG_MODE_PATTERN;
      data_neg_sgn.Which = _7SEG_D5_INDEX;
      data_neg_sgn.Value = 0x40;
      ioctl(fd, _7SEG_IOCTL_SET, &data_neg_sgn);
      break;
  }
}

void print_led(int n){

  long long int binaryNumber = 0;
  int remainder, i = 1, step = 1;
  int arr[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  n = abs(n);

  // get the binary number of n
  while (n != 0){
    remainder = n%2;
    step++;
    n /= 2;
    binaryNumber += remainder*i;
    i *= 10;
  }
  printf("binary = %d\n", binaryNumber);
  printf("\n");

  // put binary number into array
  for (i = 0; i < 8; i++){
    //printf("binary pa 10 = %d\n", binaryNumber%10);
    arr[7-i] = binaryNumber%10;
    binaryNumber = binaryNumber/10;
  }

  // determine which led should be turned on
  for (i = 0; i < 8; i++){
    //printf("%d", arr[i]);
    led = i;
    if (arr[i] == 1){
      ioctl(fd, LED_IOCTL_BIT_SET, &led);
    }
    else{
      ioctl(fd, LED_IOCTL_BIT_CLEAR, &led);
    }
  }
  printf("\n");
}

int main(int argc , char *argv[]){

  if ((fd = open("/dev/lcd", O_RDWR)) < 0){
    printf("Open /dev/lcd failed.\n");
    exit(-1);
  }

  // clear data on keypad, LCD, 7-segment, led
  ioctl(fd, KEY_IOCTL_CLEAR, key);
  ioctl(fd, LCD_IOCTL_CLEAR, NULL);
  data.Which = _7SEG_ALL;
  data.Value = 0;
  ioctl(fd, _7SEG_IOCTL_SET, &data);
  ioctl(fd, _7SEG_IOCTL_ON, NULL);
  led = LED_ALL_OFF;
  ioctl(fd, LED_IOCTL_SET, &led);

  char c[20];
  char c_new[20];
  int index_c = 0;

  while(1){
    ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
    // got something from keypad
    if (ret != -1){
      c[index_c] = change_operator(key);
      display.Count = sprintf((char *) display.Msg, "%c", c[index_c]);
      ioctl(fd, LCD_IOCTL_WRITE, &display);
      index_c++;

      if (key == '#'){
        // convert char array
        int i = 0;
        int i_new = 0;
        int tmp = 0;
        for(i = 0; i < index_c; i++){
          if (c[i] != '+' && c[i] != '-' && c[i] != '*' && c[i] != '/' && c[i] != '='){
            tmp = tmp*10 + (c[i] - '0');
          }
          else{
            c_new[i_new] = tmp + '0';
            i_new++;
            c_new[i_new] = c[i];
            i_new++;
            tmp = 0;
          }
        }

        // calculate answer
        ans = calculator(c_new, index_c);

        // print answer on LCD
        display.Count = sprintf((char *) display.Msg, "%d", ans);
        ioctl(fd, LCD_IOCTL_WRITE, &display);

        //print answer on 7-segment
        if (ans >= 0){
          print_positive(ans);
          //printf("print positive answer !\n");
        }
        else{
          print_negative(ans);
          //printf("print negative answer !\n");
        }

        // print answer on led
        print_led(ans);
      }
      else if (key == '*'){
        index_c = 0;
        data.Mode = _7SEG_MODE_HEX_VALUE;
        data.Which = _7SEG_ALL;
        data.Value = 0;
        ioctl(fd, _7SEG_IOCTL_SET, &data);
        ioctl(fd, LCD_IOCTL_CLEAR, NULL);
        led = LED_ALL_OFF;
        ioctl(fd, LED_IOCTL_SET, &led);
      }
    }
  }

  close(fd);
  return 0;
}
