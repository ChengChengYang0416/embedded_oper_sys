/*
 *  linux/drivers/i2c/chips/rtc8564.c
 *
 *  Copyright (C) 2002-2004 Stefan Eletzhofer
 *
 *	based on linux/drivers/acron/char/pcf8583.c
 *  Copyright (C) 2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Driver for system3's EPSON RTC 8564 chip
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/rtc.h>		/* get the user-level API */
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#include "rtc8564.h"

#ifdef CONFIG_MACH_CREATOR_PXA270
#include "asm/arch/lib/creator_pxa270_core.h"
#endif

#ifdef DEBUG
# define _DBG(x, fmt, args...) do{ if (debug>=x) printk(KERN_DEBUG"%s: " fmt "\n", __FUNCTION__, ##args); } while(0);
#else
# define _DBG(x, fmt, args...) do { } while(0);
#endif

#define _DBGRTCTM(x, rtctm) if (debug>=x) printk("%s: secs=%d, mins=%d, hours=%d, mday=%d, " \
			"mon=%d, year=%d, wday=%d VL=%d\n", __FUNCTION__, \
			(rtctm).tm_sec, (rtctm).tm_min, (rtctm).tm_hour, (rtctm).tm_mday, \
			(rtctm).tm_mon, (rtctm).tm_year, (rtctm).tm_wday, (rtctm).tm_isdst);

struct rtc8564_data {
	struct i2c_client client;
	u16 ctrl;
};

static inline u8 _rtc8564_ctrl1(struct i2c_client *client)
{
	struct rtc8564_data *data = i2c_get_clientdata(client);
	return data->ctrl & 0xff;
}
static inline u8 _rtc8564_ctrl2(struct i2c_client *client)
{
	struct rtc8564_data *data = i2c_get_clientdata(client);
	return (data->ctrl & 0xff00) >> 8;
}

#define CTRL1(c) _rtc8564_ctrl1(c)
#define CTRL2(c) _rtc8564_ctrl2(c)

#define BCD_TO_BIN(val) (((val)&15) + ((val)>>4)*10)
#define BIN_TO_BCD(val) ((((val)/10)<<4) + (val)%10)

static int debug=0;
module_param(debug, int, S_IRUGO | S_IWUSR);

static struct i2c_driver rtc8564_driver;
struct i2c_client *rtc8564_i2c_client = 0;

static unsigned short ignore[] = { I2C_CLIENT_END };
static unsigned short normal_addr[] = { 0x51, I2C_CLIENT_END };

static struct i2c_client_address_data addr_data = {
	.normal_i2c		= normal_addr,
	.probe			= ignore,
	.ignore			= ignore,
};
static spinlock_t epson8564_rtc_lock = SPIN_LOCK_UNLOCKED;


static int rtc8564_read_mem(struct i2c_client *client, struct mem *mem);
static int rtc8564_write_mem(struct i2c_client *client, struct mem *mem);



static int rtc8564_read(struct i2c_client *client, unsigned char adr,
			unsigned char *buf, unsigned char len)
{
	int ret = -EIO;
	unsigned char addr[1] = { adr };
	struct i2c_msg msgs[2] = {
		{client->addr, 0, 1, addr},
		{client->addr, I2C_M_RD, len, buf}
	};

	_DBG(1, "client=%p, adr=%d, buf=%p, len=%d", client, adr, buf, len);

	if (!buf) {
		ret = -EINVAL;
		goto done;
	}

	ret = i2c_transfer(client->adapter, msgs, 2);
	if (ret == 2) {
		ret = 0;
	}

done:
	return ret;
}

static int rtc8564_write(struct i2c_client *client, unsigned char adr,
			 unsigned char *data, unsigned char len)
{
	int ret = 0;
	unsigned char _data[16];
	struct i2c_msg wr;
	int i;

	if (!data || len > 15) {
		ret = -EINVAL;
		goto done;
	}

	_DBG(1, "client=%p, adr=%d, buf=%p, len=%d", client, adr, data, len);

	_data[0] = adr;
	for (i = 0; i < len; i++) {
		_data[i + 1] = data[i];
		_DBG(5, "data[%d] = 0x%02x (%d)", i, data[i], data[i]);
	}

	wr.addr = client->addr;
	wr.flags = 0;
	wr.len = len + 1;
	wr.buf = _data;

	ret = i2c_transfer(client->adapter, &wr, 1);
	if (ret == 1) {
		ret = 0;
	}

done:
	return ret;
}

static int rtc8564_attach(struct i2c_adapter *adap, int addr, int kind)
{
	int ret;
	struct i2c_client *new_client;
	struct rtc8564_data *d;
	unsigned char data[10];
	unsigned char ad[1] = { 0 };
	struct i2c_msg ctrl_wr[1] = {
		{addr, 0, 2, data}
	};
	struct i2c_msg ctrl_rd[2] = {
		{addr, 0, 1, ad},
		{addr, I2C_M_RD, 2, data}
	};

	d = kzalloc(sizeof(struct rtc8564_data), GFP_KERNEL);
	if (!d) {
		ret = -ENOMEM;
		goto done;
	}
	new_client = &d->client;

	strlcpy(new_client->name, "RTC8564", I2C_NAME_SIZE);
	i2c_set_clientdata(new_client, d);
	new_client->flags = I2C_CLIENT_ALLOW_USE;
	new_client->addr = addr;
	new_client->adapter = adap;
	new_client->driver = &rtc8564_driver;

	_DBG(1, "client=%p", new_client);

	/* init ctrl1 reg */
	data[0] = 0;
	data[1] = 0;
	ret = i2c_transfer(new_client->adapter, ctrl_wr, 1);
	if (ret != 1) {
		printk(KERN_INFO "rtc8564: cant init ctrl1\n");
		ret = -ENODEV;
		goto done;
	}

	/* read back ctrl1 and ctrl2 */
	ret = i2c_transfer(new_client->adapter, ctrl_rd, 2);
	if (ret != 2) {
		printk(KERN_INFO "rtc8564: cant read ctrl\n");
		ret = -ENODEV;
		goto done;
	}

	d->ctrl = data[0] | (data[1] << 8);

	_DBG(1, "RTC8564_REG_CTRL1=%02x, RTC8564_REG_CTRL2=%02x",
	     data[0], data[1]);

    rtc8564_i2c_client = new_client;
	ret = i2c_attach_client(new_client);
done:
	if (ret) {
		kfree(d);
	}
	return ret;
}

static int rtc8564_probe(struct i2c_adapter *adap)
{
	return i2c_probe(adap, &addr_data, rtc8564_attach);
}

static int rtc8564_detach(struct i2c_client *client)
{
	i2c_detach_client(client);
	kfree(i2c_get_clientdata(client));
	return 0;
}

static int rtc8564_get_datetime(struct i2c_client *client, struct rtc_time *dt)
{
	int ret = -EIO;
	unsigned char buf[15];

	_DBG(1, "client=%p, dt=%p", client, dt);

	if (!dt)
		return -EINVAL;

	memset(buf, 0, sizeof(buf));

	ret = rtc8564_read(client, 0, buf, 15);
	if (ret)
		return ret;

	/* century stored in minute alarm reg */
	dt->tm_year = BCD_TO_BIN(buf[RTC8564_REG_YEAR]);
	dt->tm_year += 100 * BCD_TO_BIN(buf[RTC8564_REG_AL_MIN] & 0x3f);

	dt->tm_mday = BCD_TO_BIN(buf[RTC8564_REG_DAY] & 0x3f) ;
	dt->tm_wday = BCD_TO_BIN(buf[RTC8564_REG_WDAY] & 7);
	/* dt->tm_mon is zero-based */	
	dt->tm_mon = BCD_TO_BIN(buf[RTC8564_REG_MON_CENT] & 0x1f) - 1;

	dt->tm_sec = BCD_TO_BIN(buf[RTC8564_REG_SEC] & 0x7f);
	dt->tm_isdst = (buf[RTC8564_REG_SEC] & 0x80) == 0x80;
	dt->tm_min = BCD_TO_BIN(buf[RTC8564_REG_MIN] & 0x7f);
	dt->tm_hour = BCD_TO_BIN(buf[RTC8564_REG_HR] & 0x3f);

	_DBGRTCTM(2, *dt);

	return 0;
}

static int
rtc8564_set_datetime(struct i2c_client *client, struct rtc_time *dt, int datetoo)
{
	int ret, len = 5;
	unsigned char buf[15];

	_DBG(1, "client=%p, dt=%p", client, dt);

	if (!dt)
		return -EINVAL;

	_DBGRTCTM(2, *dt);

	buf[RTC8564_REG_CTRL1] = CTRL1(client) | RTC8564_CTRL1_STOP;
	buf[RTC8564_REG_CTRL2] = CTRL2(client);
	buf[RTC8564_REG_SEC] = BIN_TO_BCD(dt->tm_sec);
	buf[RTC8564_REG_MIN] = BIN_TO_BCD(dt->tm_min);
	buf[RTC8564_REG_HR] = BIN_TO_BCD(dt->tm_hour);

	if (datetoo) {
		len += 5;
		buf[RTC8564_REG_DAY] = BIN_TO_BCD(dt->tm_mday) ;
		buf[RTC8564_REG_WDAY] = BIN_TO_BCD(dt->tm_wday);
	    /* dt->tm_mon is zero-based */			
		buf[RTC8564_REG_MON_CENT] = (BIN_TO_BCD(dt->tm_mon+1) & 0x1f);
		/* century stored in minute alarm reg */
		buf[RTC8564_REG_YEAR] = BIN_TO_BCD(dt->tm_year % 100);
		buf[RTC8564_REG_AL_MIN] = BIN_TO_BCD(dt->tm_year / 100);
	}

	ret = rtc8564_write(client, 0, buf, len);
	if (ret) {
		_DBG(1, "error writing data! %d", ret);
	}

	buf[RTC8564_REG_CTRL1] = CTRL1(client);
	ret = rtc8564_write(client, 0, buf, 1);
	if (ret) {
		_DBG(1, "error writing data! %d", ret);
	}

	return ret;
}

static int rtc8564_get_ctrl(struct i2c_client *client, unsigned int *ctrl)
{
	struct rtc8564_data *data = i2c_get_clientdata(client);

	if (!ctrl)
		return -1;

	*ctrl = data->ctrl;
	return 0;
}

static int rtc8564_set_ctrl(struct i2c_client *client, unsigned int *ctrl)
{
	struct rtc8564_data *data = i2c_get_clientdata(client);
	unsigned char buf[2];

	if (!ctrl)
		return -1;

	buf[0] = *ctrl & 0xff;
	buf[1] = (*ctrl & 0xff00) >> 8;
	data->ctrl = *ctrl;

	return rtc8564_write(client, 0, buf, 2);
}

static int rtc8564_read_mem(struct i2c_client *client, struct mem *mem)
{

	if (!mem)
		return -EINVAL;

	return rtc8564_read(client, mem->loc, mem->data, mem->nr);
}

static int rtc8564_write_mem(struct i2c_client *client, struct mem *mem)
{

	if (!mem)
		return -EINVAL;

	return rtc8564_write(client, mem->loc, mem->data, mem->nr);
}

static int
rtc8564_command(struct i2c_client *client, unsigned int cmd, void *arg)
{

	_DBG(1, "cmd=%d", cmd);

	switch (cmd) {
	case RTC_GETDATETIME:
		return rtc8564_get_datetime(client, arg);

	case RTC_SETTIME:
		return rtc8564_set_datetime(client, arg, 0);

	case RTC_SETDATETIME:
		return rtc8564_set_datetime(client, arg, 1);

	case RTC_GETCTRL:
		return rtc8564_get_ctrl(client, arg);

	case RTC_SETCTRL:
		return rtc8564_set_ctrl(client, arg);

	case MEM_READ:
		return rtc8564_read_mem(client, arg);

	case MEM_WRITE:
		return rtc8564_write_mem(client, arg);

	default:
		return -EINVAL;
	}
}

static struct i2c_driver rtc8564_driver = {
	.owner		= THIS_MODULE,
	.name		= "RTC8564",
	.id		= I2C_DRIVERID_RTC8564,
	.flags		= I2C_DF_NOTIFY,
	.attach_adapter = rtc8564_probe,
	.detach_client	= rtc8564_detach,
	.command	= rtc8564_command
};

static int epson8564_rtc_ioctl( struct inode *, struct file *, unsigned int, unsigned long);
static int epson8564_rtc_open(struct inode *inode, struct file *file);
static int epson8564_rtc_release(struct inode *inode, struct file *file);

static struct file_operations rtc_fops = {
	owner:		THIS_MODULE,
	ioctl:		epson8564_rtc_ioctl,
	open:		epson8564_rtc_open,
	release:	epson8564_rtc_release,
};

static struct miscdevice epson8564_rtc_miscdev = {
	RTC_MINOR,
	"rtc",
	&rtc_fops
};


static int
epson8564_rtc_open(struct inode *inode, struct file *file)
{
	return 0;
}

static int
epson8564_rtc_release(struct inode *inode, struct file *file)
{
	return 0;
}

static int
epson8564_rtc_ioctl( struct inode *inode, struct file *file,
		unsigned int cmd, unsigned long arg)
{
	unsigned long	flags;
	struct rtc_time wtime;
	int status = 0;

	//DPRINTK(2, "%s: cmd %c %x\n", __FUNCTION__, (char)(cmd>>8)&0xff, cmd&0xff);

	switch (cmd) {
		default:
		case RTC_UIE_ON:
		case RTC_UIE_OFF:
		case RTC_PIE_ON:
		case RTC_PIE_OFF:
		case RTC_AIE_ON:
		case RTC_AIE_OFF:
		case RTC_ALM_SET:
		case RTC_ALM_READ:
		case RTC_IRQP_READ:
		case RTC_IRQP_SET:
		case RTC_EPOCH_READ:
		case RTC_EPOCH_SET:
		case RTC_WKALM_SET:
		case RTC_WKALM_RD:
			status = -EINVAL;
			break;

		case RTC_RD_TIME:
			spin_lock_irqsave(&epson8564_rtc_lock, flags);
			rtc8564_command( rtc8564_i2c_client, RTC_GETDATETIME, &wtime);
			spin_unlock_irqrestore(&epson8564_rtc_lock,flags);

			if( copy_to_user((void *)arg, &wtime, sizeof (struct rtc_time)))
				status = -EFAULT;
			break;

		case RTC_SET_TIME:
			if (!capable(CAP_SYS_TIME))
			{
				status = -EACCES;
				break;
			}

			if (copy_from_user(&wtime, (struct rtc_time *)arg, sizeof(struct rtc_time)) )
			{
				status = -EFAULT;
				break;
			}

			spin_lock_irqsave(&epson8564_rtc_lock, flags);
			rtc8564_command( rtc8564_i2c_client, RTC_SETDATETIME, &wtime);
			spin_unlock_irqrestore(&epson8564_rtc_lock,flags);
			break;
	}

	return status;
}
#if 0
static char *
epson8564_mon2str( unsigned int mon)
{
	char *mon2str[12] = {
	  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	if( mon > 11) return "error";
	else return mon2str[ mon];
}


static int epson8564_rtc_proc_output( char *buf)
{
#define CHECK(ctrl,bit) ((ctrl & bit) ? "yes" : "no")
	unsigned char ram[epson8564_RAM_SIZE];
	int ret;

	char *p = buf;

	ret = epson8564_readram( ram, epson8564_RAM_SIZE);
	if( ret > 0)
	{
		int i;
		struct rtc_time dt;
		char text[9];

		p += sprintf(p, "DS1307 (64x8 Serial Real Time Clock)\n");

		epson8564_convert_to_time( &dt, ram);
		p += sprintf(p, "Date/Time	     : %02d-%s-%04d %02d:%02d:%02d\n",
			dt.tm_mday, epson8564_mon2str(dt.tm_mon), dt.tm_year + 1900,
			dt.tm_hour, dt.tm_min, dt.tm_sec);

		p += sprintf(p, "Clock halted	     : %s\n", CHECK(ram[0],0x80));
		p += sprintf(p, "24h mode	     : %s\n", CHECK(ram[2],0x40));
		p += sprintf(p, "Square wave enabled : %s\n", CHECK(ram[7],0x10));
		p += sprintf(p, "Freq		     : ");

		switch( ram[7] & 0x03)
		{
			case RTC8564_TD_1HZ:
				p += sprintf(p, "1Hz\n");
				break;
			case RTC8564_TD_4096HZ:
				p += sprintf(p, "4.096kHz\n");
				break;
			case RTC8564_TD_8192HZ:
				p += sprintf(p, "8.192kHz\n");
				break;
			case RTC8564_TD_32768HZ:
			default:
				p += sprintf(p, "32.768kHz\n");
				break;

		}

		p += sprintf(p, "RAM dump:\n");
		text[8]='\0';
		for( i=0; i<epson8564_RAM_SIZE; i++)
		{
			p += sprintf(p, "%02X ", ram[i]);

			if( (ram[i] < 32) || (ram[i]>126)) ram[i]='.';
			text[i%8] = ram[i];
			if( (i%8) == 7) p += sprintf(p, "%s\n",text);
		}
		p += sprintf(p, "\n");
	}
	else
	{
		p += sprintf(p, "Failed to read RTC memory!\n");
	}

	return	p - buf;
}

static int epson8564_rtc_read_proc(char *page, char **start, off_t off,
		int count, int *eof, void *data)
{
	int len = epson8564_rtc_proc_output (page);
	if (len <= off+count) *eof = 1;
	*start = page + off;
	len -= off;
	if (len>count) len = count;
	if (len<0) len = 0;
	return len;
}
#endif

static __init int rtc8564_init(void)
{
    int retval=0;	

#ifdef CONFIG_MACH_CREATOR_PXA270
       if (creator_GetCreatorCPLDVersion() == CREATOR_BOARD_NOT_EXIST){
            printk("Warning : Not support rtc8564 because of Creator board not present.\n");        	    
       	    return (-1);
       }
#endif	
    
	retval =  i2c_add_driver(&rtc8564_driver);
	if (retval==0)
	{
		misc_register (&epson8564_rtc_miscdev);
		//create_proc_read_entry (PROC_epson8564_NAME, 0, 0, epson8564_rtc_read_proc, NULL);
		printk("I2C: Epson 8564 RTC driver successfully loaded\n");
		//if (rtc_debug > 2) epson8564_dumpram();
	}
	return retval;	
}

static __exit void rtc8564_exit(void)
{
	misc_deregister(&epson8564_rtc_miscdev);	
	i2c_del_driver(&rtc8564_driver);
}

MODULE_AUTHOR("Stefan Eletzhofer <Stefan.Eletzhofer@eletztrick.de>");
MODULE_DESCRIPTION("EPSON RTC8564 Driver");
MODULE_LICENSE("GPL");

module_init(rtc8564_init);
module_exit(rtc8564_exit);
