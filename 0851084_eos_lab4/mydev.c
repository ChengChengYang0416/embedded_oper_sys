#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");

char rbuf_0[8] = {1, 1, 1, 1, 1, 1, 0};
char rbuf_1[8] = {0, 1, 1, 0, 0, 0, 0};
char rbuf_2[8] = {1, 1, 0, 1, 1, 0, 1};
char rbuf_3[8] = {1, 1, 1, 1, 0, 0, 1};
char rbuf_4[8] = {0, 1, 1, 0, 0, 1, 1};
char rbuf_5[8] = {1, 0, 1, 1, 0, 1, 1};
char rbuf_6[8] = {1, 0, 1, 1, 1, 1, 1};
char rbuf_7[8] = {1, 1, 1, 0, 0, 0, 0};
char rbuf_8[8] = {1, 1, 1, 1, 1, 1, 1};
char rbuf_9[8] = {1, 1, 1, 1, 0, 1, 1};

char kbuf;

static ssize_t my_read(struct file *fp, char *buf, size_t count, loff_t *fpos){
  printk("call read\n");

  int ret = 0;
  char rbuf[7] = {0, 0, 0, 0, 0, 0, 0};

  int i = 0;
  switch ((int)(kbuf - '0')){
    case 0:
      for (i = 0; i < 7; i++){
        rbuf[i] = rbuf_0[i];

      }
      break;

    case 1:
      for (i = 0; i < 7; i++){
        rbuf[i] = rbuf_1[i];

      }
      break;

    case 2:
      for (i = 0; i < 7; i++){
        rbuf[i] = rbuf_2[i];

      }
      break;

    case 3:
      for (i = 0; i < 7; i++){
        rbuf[i] = rbuf_3[i];

      }
      break;

    case 4:
      for (i = 0; i < 7; i++){
        rbuf[i] = rbuf_4[i];

      }
      break;

    case 5:
      for (i = 0; i < 7; i++){
        rbuf[i] = rbuf_5[i];

      }
      break;

    case 6:
      for (i = 0; i < 7; i++){
        rbuf[i] = rbuf_6[i];

      }
      break;

    case 7:
      for (i = 0; i < 7; i++){
        rbuf[i] = rbuf_7[i];

      }
      break;

    case 8:
      for (i = 0; i < 7; i++){
        rbuf[i] = rbuf_8[i];
      }
      break;

    case 9:
      for (i = 0; i < 7; i++){
        rbuf[i] = rbuf_9[i];

      }
      break;
   }

  printk("%01x %01x %01x %01x %01x %01x %01x", rbuf[0] , rbuf[1] , rbuf[2] , rbuf[3] , rbuf[4] , rbuf[5] , rbuf[6]);

  ret = copy_to_user(buf, &rbuf, sizeof(rbuf));

  return ret;
}


static ssize_t my_write(struct file *fp, const char *buf, size_t count, loff_t *fpos){

  //printk("call write!\n");

  int ret = 0;
  ret = copy_from_user(&kbuf, buf, 1);
  //printk("%01x\n", (kbuf - '0'));

  return ret;
}

static int my_open(struct inode *inode, struct file *fp){
  printk("call open\n");
  return 0;
}

struct file_operations my_fops = {
  read: my_read,
  write: my_write,
  open: my_open
};

#define MAJOR_NUM 244
#define DEVICE_NAME "my_dev"

static int my_init(void){
  printk("call init\n");
  if(register_chrdev(MAJOR_NUM, DEVICE_NAME, &my_fops) < 0){
    printk("Can not get major %d\n", MAJOR_NUM);
    return (-EBUSY);
  }

  printk("My device is started and the major is %d\n", MAJOR_NUM);
  return 0;
}

static void my_exit(void){
  unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
  printk("call exit\n");
}


module_init(my_init);
module_exit(my_exit);
