#include "asm-arm/arch-pxa/lib/creator_pxa270_lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>


// declare for receive inventory
#define inv_row 20
#define inv_col 20
char inventory[inv_row][inv_col];

typedef struct Inv{
    int line, a, b, c;
    char city[50];
} inv;

inv data[5][100];
int in_num[5];
char s[100];
int num;


// declare for checking whether login or logout
bool inOut = false;

// declare for ID number
char ID_number[10];
char ID_number_arr[10][10];
int i_ID = 0;
int day_of_week = 0;
bool allowlog = false;
bool bought = false;
int buy_n = 0;
int bought_n = 0;
int login_n = 0;
typedef struct Buy{
  char buy_ID[10];
  int ifbought;
}checkbuy;
checkbuy check[10];

// declare for buy
char buyinfo[10];
int id = 0;
char type = 0;
int type_n = 0;
int mask_carbon = 0;
int mask_medical = 0;
int mask_N95 = 0;

// declare for alcohol
int alcohol = 5;

// declare for 7 segment
_7seg_info_t seg;

// declare for led
unsigned short led;

void readInventory(int fd_inv, lcd_write_info_t display_inv){

  ioctl(fd_inv, LCD_IOCTL_CLEAR, NULL);

  FILE *fp = fopen("inventory.txt", "r");
  if (fp == NULL){
    perror ("Error opening file");
  }

  /**
  INPUT
  **/

  fgets(s, 100, fp);
  sscanf(s, "%d", &num);
  printf("%d\n", num);
  display_inv.Count = sprintf((char *) display_inv.Msg, "%d\n", num);
  ioctl(fd_inv, LCD_IOCTL_WRITE, &display_inv);
  int i = 0;
  for (i = 0; i < num; ++i) {
    char type;
    fgets(s, 100, fp);
    sscanf(s, "%c", &type);
    printf("%c\n", type);
    display_inv.Count = sprintf((char *) display_inv.Msg, "%c\n", type);
    ioctl(fd_inv, LCD_IOCTL_WRITE, &display_inv);

    fgets(s, 100, fp);
    sscanf(s, "%d", &(in_num[i]));
    printf("%d\n", in_num[i]);
    display_inv.Count = sprintf((char *) display_inv.Msg, "%d\n", in_num[i]);
    ioctl(fd_inv, LCD_IOCTL_WRITE, &display_inv);
    int j = 0;
    for (j = 0; j < in_num[i]; ++j)
    {
      fgets(s, 100, fp);
      sscanf(s, "%d %s %d %d %d", &(data[i][j].line), data[i][j].city, &(data[i][j].a), &(data[i][j].b), &(data[i][j].c));
      printf("%d %s %d %d %d\n", data[i][j].line, data[i][j].city, data[i][j].a, data[i][j].b, data[i][j].c);
      display_inv.Count = sprintf((char *) display_inv.Msg, "%d %s %d %d %d\n", data[i][j].line, data[i][j].city, data[i][j].a, data[i][j].b, data[i][j].c);
      ioctl(fd_inv, LCD_IOCTL_WRITE, &display_inv);
    }
  }
  fclose(fp);

}

void rewriteInv(int fd_re, lcd_write_info_t display_re){

  //ioctl(fd_re, LCD_IOCTL_CLEAR, NULL);
  FILE *outfp = fopen("inventory.txt", "w");
  fprintf(outfp, "%d\n", num);
  printf("%d\n", num);
  display_re.Count = sprintf((char *) display_re.Msg, "%d\n", num);
  //ioctl(fd_re, LCD_IOCTL_WRITE, &display_re);

  int i = 0;
  for (i = 0; i < num; ++i) {
    fprintf(outfp, "%c\n", i + 'A'); // print type
    printf("%c\n", i + 'A'); // print type
    display_re.Count = sprintf((char *) display_re.Msg, "%c\n", i + 'A');
    //ioctl(fd_re, LCD_IOCTL_WRITE, &display_re);

    fprintf(outfp, "%d\n", in_num[i]);
    printf("%d\n", in_num[i]);
    display_re.Count = sprintf((char *) display_re.Msg, "%d\n", in_num[i]);
    //ioctl(fd_re, LCD_IOCTL_WRITE, &display_re);

    int j = 0;
    for (j = 0; j < in_num[i]; ++j)
    {
      fprintf(outfp, "%d %s %d %d %d\n", j+1, data[i][j].city, data[i][j].a, data[i][j].b, data[i][j].c);
      printf("%d %s %d %d %d\n", j+1, data[i][j].city, data[i][j].a, data[i][j].b, data[i][j].c);
      display_re.Count = sprintf((char *) display_re.Msg, "%d %s %d %d %d\n", j+1, data[i][j].city, data[i][j].a, data[i][j].b, data[i][j].c);
      //ioctl(fd_re, LCD_IOCTL_WRITE, &display_re);
    }
  }

  fclose(outfp);
}

void checkIDnumber(){
  printf("check\n");
  int ID[10];
  int i = 0;
  int j = 0;
  int z = 0;
  int y = 0;
  bool A = false;
  bool B = false;
  for (i = 0; i < 10; i++){
    ID[i] = (int)(ID_number[i] - '0');
  }
  switch (ID_number[0]){
  case 'A':
    y = 1;
    break;
  case 'B':
    y = 10;
    break;
  case 'C':
    y = 19;
    break;
  case 'D':
    y = 28;
  }
  z = 10 - (y + ID[1]*8 + ID[2]*7 + ID[3]*6 + ID[4]*5 + ID[5]*4 + ID[6]*3 + ID[7]*2 + ID[8]*1)%10;
  if (z == ID[9]){
    A = true;
  }
  else {
    A = false;
  }

  if (day_of_week == 7){
    B = true;
  }
  else{
    if (day_of_week%2 == ID[9]%2){
      B = true;
    }
    else{
      B = false;
    }
  }
  if (A == true && B == true){
    allowlog = true;
  }
  else{
    allowlog = false;
  }
  /*
  if (i_ID != 0){
    for (i = 0; i < 10; i++){
      for (j = 0; j < 10; j++){
        if(check[i].buy_ID[j] = ID_number[j]){

        }
      }

    }
  }*/
}

void print_7seg(int fd_7seg){
  int n = 1000*(ID_number[6] - '0')+100*(ID_number[7] - '0')+10*(ID_number[8] - '0')+(ID_number[9] - '0');
  char hex[10];
  sprintf(hex, "%d", n);
  int num = (int)strtol(hex, NULL, 16);
  seg.Mode = _7SEG_MODE_HEX_VALUE;
  seg.Which = _7SEG_ALL;
  seg.Value = num;
  ioctl(fd_7seg, _7SEG_IOCTL_SET, &seg);
}

void print_led(int fd_led, int n){

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
  //printf("binary = %d\n", binaryNumber);
  //printf("\n");

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
      ioctl(fd_led, LED_IOCTL_BIT_SET, &led);
    }
    else{
      ioctl(fd_led, LED_IOCTL_BIT_CLEAR, &led);
    }
  }
  //printf("\n");
}


void login(int fd_login){
  int ret_login;
  char key_login;
  int i_login = 0;
  lcd_write_info_t display_login;
  ioctl(fd_login, KEY_IOCTL_CLEAR, key_login);
  while (1){
    ret_login = ioctl(fd_login, KEY_IOCTL_GET_CHAR, &key_login);
    if (ret_login == 0){
      if (key_login == '#'){
        checkIDnumber();
        if (allowlog){
          int i = 0;
          for (i = 0; i < 10; i++){
            check[i_ID].buy_ID[i] = ID_number[i];
            check[i_ID].ifbought++;
          }
          i_ID++;
          if (i_ID >= 10){
            i_ID = 0;
          }
          login_n++;
          print_7seg(fd_login);
          printf("Login successfully\n.");
          ioctl(fd_login, LCD_IOCTL_CLEAR, NULL);
          display_login.Count = sprintf((char *) display_login.Msg, "\nLogin successfully.\n");
          ioctl(fd_login, LCD_IOCTL_WRITE, &display_login);
          display_login.Count = sprintf((char *) display_login.Msg, "Press 3 to buy the mask.\n");
          ioctl(fd_login, LCD_IOCTL_WRITE, &display_login);
          break;
        }
        else {
          if (bought == true){
            ioctl(fd_login, LCD_IOCTL_CLEAR, NULL);
            display_login.Count = sprintf((char *) display_login.Msg, "\nYou have already bought masks.\n");
            ioctl(fd_login, LCD_IOCTL_WRITE, &display_login);
            sleep(3);
          }
          printf("You are not allowed to login\n");
          ioctl(fd_login, LCD_IOCTL_CLEAR, NULL);
          display_login.Count = sprintf((char *) display_login.Msg, "\nYou are not allowed to login.\n");
          ioctl(fd_login, LCD_IOCTL_WRITE, &display_login);
          display_login.Count = sprintf((char *) display_login.Msg, "Press 2 to login again.\n");
          ioctl(fd_login, LCD_IOCTL_WRITE, &display_login);
          sleep(2);
          break;
        }
      }
      else{
        ID_number[i_login] = key_login;
        //ID_number_arr[i_ID][i_login] = key_login;
        //check[i_ID].buy_ID[i_login] = key_login;
        display_login.Count = sprintf((char *) display_login.Msg, "%c", ID_number[i_login]);
        ioctl(fd_login, LCD_IOCTL_WRITE, &display_login);
        i_login++;
      }
    }
  }
}

void buy(int fd_buy){
  int ret_buy;
  char key_buy;
  int i_buy = 0;
  lcd_write_info_t display_buy;
  ioctl(fd_buy, KEY_IOCTL_CLEAR, key_buy);

  while(1){
    ret_buy = ioctl(fd_buy, KEY_IOCTL_GET_CHAR, &key_buy);
    if (ret_buy == 0){
      if (key_buy == '#'){
        type = buyinfo[0];
        type_n = (int)(buyinfo[2] - '0');
        mask_carbon = (int)(buyinfo[4] - '0');
        mask_medical = (int)(buyinfo[6] - '0');
        mask_N95 = (int)(buyinfo[8] - '0');
        if (i_buy != 9 || (mask_carbon+mask_medical+mask_N95) > 3){
          display_buy.Count = sprintf((char *) display_buy.Msg, "\nYou are buying too much masks.\nPress 3 to key in again.\n");
          ioctl(fd_buy, LCD_IOCTL_WRITE, &display_buy);
          break;
        }
        if (buy_n >=1){
          buy_n = 0;
          display_buy.Count = sprintf((char *) display_buy.Msg, "\nYou have bought masks.\nPress 2 to logout\n");
          ioctl(fd_buy, LCD_IOCTL_WRITE, &display_buy);
          break;
        }
        else{
          bought_n++;
          buy_n++;
          data[type - 'A'][type_n-1].a -= mask_carbon;
          data[type - 'A'][type_n-1].b -= mask_medical;
          data[type - 'A'][type_n-1].c -= mask_N95;
          if (data[type - 'A'][type_n-1].a <= 0){
            data[type - 'A'][type_n-1].a = 0;
          }
          if (data[type - 'A'][type_n-1].b <= 0){
            data[type - 'A'][type_n-1].b = 0;
          }
          if (data[type - 'A'][type_n-1].b <= 0){
            data[type - 'A'][type_n-1].b = 0;
          }

          display_buy.Count = sprintf((char *) display_buy.Msg, "\nDisinfecting hands with alcohol.\n");
          ioctl(fd_buy, LCD_IOCTL_WRITE, &display_buy);
          if (alcohol <= 0){
            alcohol = 5;
            display_buy.Count = sprintf((char *) display_buy.Msg, "Wait 5 secs for alcohol supplement.\n");
            ioctl(fd_buy, LCD_IOCTL_WRITE, &display_buy);
            sleep(5);
          }
          alcohol--;
          display_buy.Count = sprintf((char *) display_buy.Msg, "Putting masks into envelope.\n");
          ioctl(fd_buy, LCD_IOCTL_WRITE, &display_buy);
          sleep(2);
          display_buy.Count = sprintf((char *) display_buy.Msg, "Thank you for your purchase.\n");
          ioctl(fd_buy, LCD_IOCTL_WRITE, &display_buy);
          sleep(2);
          ioctl(fd_buy, LCD_IOCTL_CLEAR, NULL);
          display_buy.Count = sprintf((char *) display_buy.Msg, "1. show\n2. logout\n3. buy\n4. statistics\n");
          ioctl(fd_buy, LCD_IOCTL_WRITE, &display_buy);
          rewriteInv(fd_buy, display_buy);
          break;
        }
      }
      else {
        buyinfo[i_buy] = key_buy;
        display_buy.Count = sprintf((char *) display_buy.Msg, "%c", buyinfo[i_buy]);
        ioctl(fd_buy, LCD_IOCTL_WRITE, &display_buy);
        i_buy++;
      }

    }
  }
}

bool ifboughtfn(){
  int i = 0;
  int j = 0;
  int num_of_same = 0;
  printf("login_n = %d", login_n);
  for (i = 0; i < login_n; i++){
    for (j = 0; j < 10; j++){
      if (check[i].buy_ID[j] == ID_number[j]){
        num_of_same++;
      }
    }
    if (num_of_same >= 10){
      check[i].ifbought++;
      return true;
    }
    else{
      return false;
    }
  }
}

int main(int argc , char *argv[]){

  // check if usage is correct
  day_of_week = atoi(argv[1]);
  if (argc != 2 || day_of_week < 1 || day_of_week > 7){
    printf("Usage : ./hw1 <day_of_week>\n\n");
    exit(-1);
  }

  // declare for LCD, keypad, 7seg, led
  int fd, ret;
  lcd_write_info_t display;
  char key;

  // use the lcd driver, clear LCD, print program menu on LCD
  if ((fd = open("/dev/lcd", O_RDWR)) < 0){
    printf("Open /dev/lcd failed.\n");
    exit(-1);
  }
  ioctl(fd, LCD_IOCTL_CLEAR, NULL);
  ioctl(fd, KEY_IOCTL_CLEAR, key);
  seg.Which = _7SEG_ALL;
  seg.Value = 0;
  ioctl(fd, _7SEG_IOCTL_SET, &seg);

  display.Count = sprintf((char *) display.Msg, "1. show\n2. login\n3. buy\n4. statistics\n");
  ioctl(fd, LCD_IOCTL_WRITE, &display);

  print_led(fd, day_of_week);
  int num_buy = 0;

  while (1){
    ret = ioctl(fd, KEY_IOCTL_GET_CHAR, &key);
    if (ret == 0){
      switch (key) {
      case '1':
        readInventory(fd, display);   // read inventory from inventory.txt
        break;

      case '2':
        if (inOut == false){
          inOut = true;
          ioctl(fd, LCD_IOCTL_CLEAR, NULL);
          display.Count = sprintf((char *) display.Msg, "login\nKey in your ID number:\n");
          ioctl(fd, LCD_IOCTL_WRITE, &display);
          login(fd);
        }
        else{
          buy_n = 0;
          inOut = false;
          seg.Which = _7SEG_ALL;
          seg.Value = 0;
          ioctl(fd, _7SEG_IOCTL_SET, &seg);
          ioctl(fd, LCD_IOCTL_CLEAR, NULL);
          display.Count = sprintf((char *) display.Msg, "1. show\n2. login\n3. buy\n4. statistics\n");
          ioctl(fd, LCD_IOCTL_WRITE, &display);
        }
        break;

      case '3':
        if (inOut == false){
          ioctl(fd, LCD_IOCTL_CLEAR, NULL);
          display.Count = sprintf((char *) display.Msg, "Press 2 to login.\n");
          ioctl(fd, LCD_IOCTL_WRITE, &display);
        }
        else{
          if (bought_n >= 1){
            if (ifboughtfn()){
              ioctl(fd, LCD_IOCTL_CLEAR, NULL);
              display.Count = sprintf((char *) display.Msg, "You have already bought mask.\nCome and buy next week.\n");
              ioctl(fd, LCD_IOCTL_WRITE, &display);
            }
            else{
              ioctl(fd, LCD_IOCTL_CLEAR, NULL);
              display.Count = sprintf((char *) display.Msg, "Key in your buying info:\n");
              ioctl(fd, LCD_IOCTL_WRITE, &display);
              buy(fd);
            }
          }
          else{
            ioctl(fd, LCD_IOCTL_CLEAR, NULL);
            display.Count = sprintf((char *) display.Msg, "Key in your buying info:\n");
            ioctl(fd, LCD_IOCTL_WRITE, &display);
            buy(fd);
          }

        }
        break;
      case '4':
        ioctl(fd, LCD_IOCTL_CLEAR, NULL);
        display.Count = sprintf((char *) display.Msg, "Alcohol : %d\n", alcohol);
        ioctl(fd, LCD_IOCTL_WRITE, &display);
        break;

      default:
        break;
      }
    }
    if((key & 0xff) == '#') break;
  }

  close(fd);

  return 0;
}
