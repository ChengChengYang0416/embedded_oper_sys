// --------------------------------------------------------------------
//
//   Title     :  creator-pxa270-lcd.c
//             :
//   Library   :
//             :
//   Developers:  MICROTIME MDS group
//             :
//   Purpose   :  Driver for AF-128128CFI-H LCD(128*128 pixels, 4 colors)
//             :
//   Limitation:
//             :
//   Note      :
//             :
// --------------------------------------------------------------------
//   modification history :
// --------------------------------------------------------------------
//   Version| mod. date: |
//   Vx.x   | mm/dd/yyyy |
//   V1.00  | 04/25/2006 | First release
//   V1.01  | 06/06/2006 | 修正 Scan 7-Segment ISR, 使用io_reg0要用creator_io.io_reg0
// --------------------------------------------------------------------
//
// Note:
//
//       MICROTIME COMPUTER INC.
//
//
/*************************************************************************
Include files
*************************************************************************/
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/cdev.h>
#include <linux/config.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>

#include <linux/mm.h>
#include <linux/sched.h>	
#include <linux/timer.h>	
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <asm/irq.h>
#include <asm/param.h>	
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/uaccess.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include "asm/arch/lib/creator_pxa270_core.h"
#include "asm/arch/lib/creator_pxa270_lcd.h"
#include "asm/arch/lib/genfont8_8.h"


/*************************************************************************
Constant define
*************************************************************************/

/* ****** Debug Information ******************************************** */
#define DEBUG
#ifdef DEBUG 
#define DEBUG_INFO()	printk("%s %d\n",__FUNCTION__,__LINE__)
#define MSG(string, args...) printk("<1>" string, ##args)
#else   
#define MSG(string, args...)
#define DEBUG_INFO()	;
#endif


/* ****** Linux Information ******************************************** */
#if LINUX_VERSION_CODE < 0x020100
#define GET_USER(a,b)   a = get_user(b)
#else
#include <asm/uaccess.h>
#define GET_USER(a,b)   get_user(a,b)
#endif


/* ****** Module Information ******************************************* */
#define	MAJOR_NUM			LCD_MAJOR_NUM
#define MAX_MINORS          1

#define MODULE_VERSION_INFO	"1.01"
#define MODULE_NAME			"LCDTXT_CREATOR"
#define COPYRIGHT			"Copyright (C) 2006 Microtime Computer Inc."
#define MODULE_AUTHOR_STRING		"Microtime Computer Inc."
#define MODULE_DESCRIPTION_STRING	"Creator LCD module"


/* ****** 7-Segment Scanning time *************************************** */	
#define INT_NO				         CREATOR_OST_4_IRQ
#define	_7SEGMENT_RESCHED_PERIOD	 5		// 5ms
#define TIMER_COUNT                  _7SEGMENT_RESCHED_PERIOD



/* ****** Keypad Scanning time ****************************************** */
#define KEYPAD_SCAN_FREQ             5
#define KEYPAD_SCAN_PERIOD		     (HZ/KEYPAD_SCAN_FREQ)		// 200ms			



enum  lcd_timer {
    TTIMER_SETUP,
	TTIMER_START,
	TTIMER_STOP
};



/* ****** LCD command *************************************************** */
enum LCDCommand{
	LCD_START_OSC=0,		// Start oscillation	
	LCD_DEVICE_CODE=0,		// Device code read	
	LCD_DRV_OUT=1,			// Driver output control
	LCD_WAVEFORM=2,			// LCD-driving-waveform control
	LCD_PWR_CONTROL=3,		// Power control
	LCD_CONTRAST=4,			// Contrast control
	LCD_ENTRY_MODE=5,		// Entry mode
	LCD_ROTATION=6,			// Rotation
	LCD_DISPLAY=7,			// Display control		
	LCD_CURSOR=8,			// Cursor control
	LCD_DB_HEIGHT=9,		// Double-height display position
	LCD_VET_SCROLL=0xa,		// Vertical scroll
	LCD_HOR_CURSOR_POS=0xb,		// Horizontal cursor position
	LCD_VER_CURSOR_POS=0xc,		// Vertical cursor position
	LCD_RAM_MASK=0x10,		// RAM write data mask
	LCD_RAM_ADDR=0x11,		// RAM address set
	LCD_RAM_DATA=0x12,		// RAM Data
	LCD_RAM_WR=0x12,		// RAM data write
	LCD_RAM_RD=0x12			// RAM data read
};


// 讀寫LCD RAM的Macro 
#define SET_ADDRESS_LCD(x)		WriteLCD(LCD_RAM_ADDR, x);
#define SET_RAM_DATA_LCD(x)		WriteLCD(LCD_RAM_DATA, x);



/*************************************************************************
Function prototypes
*************************************************************************/
static unsigned short ReadLCD (unsigned short addr);
static void WriteLCD (unsigned short addr, unsigned short data);
static void LCD_ClearViewArea (void);
static void LCD_PutRAMData (unsigned short address, unsigned short data);
static void LCD_ClearScreen (void);
static void LCD_ClearOneLine (int y);
static void LCD_scroll (void);
static void LCD_displ (int x, int y, int ch);
static void LCD_putchar (char c);
static void LCD_CursorMode (unsigned char Mode);
static void LCD_EnableCursor (int bCursorOn);
static void LCD_SetCursor(int x, int y);
static void InitialLCD (void);
static void Initial_LCD (void);

static void OptimizationDelay(void) {}	// for Optimization
static void ScanTimer(void);



/*************************************************************************
Variable define
*************************************************************************/
extern creator_io_t creator_io ;

static DECLARE_WAIT_QUEUE_HEAD(wq_read);
static char* g_7_segment_timer_id = "creator 7-segment timer"; /* global 7-segment led timer id for irq */

static unsigned char       Xpos, Ypos, ScrWidth, ScrHeight,YScrollOffset;
static unsigned char       byFontHeight, byFontWidth;
static unsigned char       byCursorOn, byCursorMode;

/*	7_SEG LED */
static unsigned char  seven_seg_buf[4], seven_seg_idx;
static unsigned short tbl_7seg_com[4] = {0x0e00, 0x0d00, 0x0b00, 0x0700};

/*	KEY_PAD */
static UI scan_led=0x5500;
static UC scan_key_in_idx, scan_key_out_idx;
static UI scan_key_buf[16], last_scan_key;
static UC key_assigned;

/*	Timer system tick */
static UC tick_count;
static volatile UC tick_sec, tick_min, tick_hour;

static spinlock_t _7segment_time_lock = SPIN_LOCK_UNLOCKED;
static spinlock_t time_tick_lock = SPIN_LOCK_UNLOCKED;
/* ************************************************************************** */



void
static Delay (UI ms)
{ 
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout(((HZ*ms)/1000));           
}  
/* ************************************************************************** */



//對LCD讀取資料
static unsigned short 
ReadLCD (unsigned short addr)
{
        unsigned short data;

        LCD_CMD = 0xff;		//MSB
        OptimizationDelay();	//for Optimization	
        LCD_CMD = addr;		//LSB
        data = LCD_DATA << 8;	//MSB
        data |= LCD_DATA;	//LSB

        // 若是讀取LCD的顯示區域CGRAM資料時, 
        // 第一次資料是不合法, 第二次才是正確資料
        if (addr == LCD_RAM_DATA) {
            LCD_CMD = 0xff;		//MSB
            OptimizationDelay();	//for Optimization	    
            LCD_CMD = addr;		//LSB
            data = LCD_DATA << 8;	//MSB
            data |= LCD_DATA;		//LSB
        }
        
        return (data);
}
/* ************************************************************************** */



//對LCD設定資料
static void 
WriteLCD (unsigned short addr, unsigned short data)
{
        LCD_CMD = 0xff;			//MSB
        OptimizationDelay();	//for Optimization	
        LCD_CMD = addr;			//LSB
		
        LCD_DATA = (data >> 8) ;    //MSB
        OptimizationDelay();		//for Optimization	
        LCD_DATA = data;		    //LSB
}        
/* ************************************************************************** */



//清除LCD畫面, 並重新設定垂直捲動
static void 
LCD_ClearViewArea (void)
{
        int	i;
	
        SET_ADDRESS_LCD(0);		//設定開始寫入的LCD RAM的起始位址
        for (i=0; i < 0x800; i++)	//清除LCD畫面
            SET_RAM_DATA_LCD(0);
	    
        WriteLCD(LCD_VET_SCROLL, 0);	//重新設定垂直捲動	
}
/* ************************************************************************** */



//將資料寫入LCD的顯示區域內
static void 
LCD_PutRAMData (unsigned short address, unsigned short data)
{
        SET_ADDRESS_LCD(address);	//設定LCD顯示區域RAM的位址
        SET_RAM_DATA_LCD(data);		//將所指定的資料寫入所指定LCD顯示區域RAM
}
/* ************************************************************************** */



//清除整個LCD營幕成空白, 並且游標重新歸位為原點
static void 
LCD_ClearScreen (void)
{
        LCD_ClearViewArea();
        Xpos = Ypos = YScrollOffset = 0;
        WriteLCD(LCD_VET_SCROLL, 0);
        LCD_SetCursor(Xpos, Ypos);		
}
/* ************************************************************************** */



//將LCD某行變成空白行
static void 
LCD_ClearOneLine (int y)
{
        int             x;

        for (x=0; x < ScrWidth; x++)	
            LCD_displ(x, y, ' ');	//在X,Y寫入空白字元
        Xpos = 0;
}
/* ************************************************************************** */



//LCD往上垂直捲動一行
static void 
LCD_scroll (void)
{
        if (YScrollOffset == 0)
            YScrollOffset = ScrHeight - 1;
        else
            YScrollOffset--;

        WriteLCD(LCD_VET_SCROLL, byFontHeight * (Ypos - YScrollOffset + 1));
        LCD_ClearOneLine(Ypos);		//最下面一行清除為空白行
}
/* ************************************************************************** */



//將單一個字元寫入LCD
static void 
LCD_displ (int x, int y, int ch)
{
        unsigned short  addr, data ;
        int             i, YRelation;

        //可顯示自元範圍從0x20~0x7E
        //其餘用空白取代
        data = (ch >= 0x20 && ch <= 0x7E) ? ch : ' ';
        
        //計算字元所在位置
        YRelation = y - YScrollOffset;
        addr = x + (YRelation * 0x80);

        //取出字元的圖型資料後寫入LCD
        for (i=0; i < byFontWidth; i++){
            LCD_PutRAMData(addr, charset[data][i]);
            addr += 0x10;
        }
}
/* ************************************************************************** */



//將單一個字元寫入顯示裝置
static void 
LCD_putchar (char c)
{
        switch (c){
        case '\n' :			//換行字元 
            if (Ypos == (ScrHeight-1))	//如果到達最後一行將往上捲動一行
                LCD_scroll();
            else
                Ypos++;
            Xpos = 0;
            break ;
        case '\t' :			//TAB字元用空白字元替換
            do LCD_putchar(' ');
            while (Xpos%TABS);
            break ;
        default :			//當超過每行最大寬度時自動換行
            if ((Xpos+1) >= ScrWidth){
                LCD_putchar('\n');
            }
            LCD_displ(Xpos++, Ypos, c);	//將字元寫入LCD
            break ;
        }
        if (byCursorOn)			
            LCD_SetCursor(Xpos, Ypos);	//移動游標到所顯式的字元後面
}
/* ************************************************************************** */



//設定游標模式
static void 
LCD_CursorMode (unsigned char Mode)
{
        byCursorMode = Mode & 0x3;
}
/* ************************************************************************** */



//啟動/關閉游標
static void 
LCD_EnableCursor (int bCursorOn)
{
        unsigned short  data;

        byCursorOn = bCursorOn;
        if (byCursorOn)
            data = byCursorMode | 0x04;
        else
            data = 0;
        WriteLCD(LCD_CURSOR, data);
}
/* ************************************************************************** */



//讀取游標位置
static void 
LCD_GetCursor (int *x, int *y)
{
        *x = Xpos ; *y = Ypos;		
}
/* ************************************************************************** */



//設定游標位置
static void 
LCD_SetCursor (int x, int y)
{
        unsigned short  HS, HE, VS;

        HS = x * byFontWidth;
        HE = HS + (byFontWidth - 1);
        WriteLCD(LCD_HOR_CURSOR_POS, (HE << 8) + HS);

        VS = (y + 1) * byFontHeight - 2;
        WriteLCD(LCD_VER_CURSOR_POS, ((VS+1)<<8) + VS);
        
        Xpos = x; Ypos = y;
}
/* ************************************************************************** */



static void 
LCD_DrawFullImage (unsigned short *pImageBuffer)
{
        int	i;
	
        SET_ADDRESS_LCD(0);
	
        for (i=0; i < 0x800; i++){
            SET_RAM_DATA_LCD(pImageBuffer[i]);
        }
}
/* ************************************************************************** */



// 初始化LCD的命令集
static void 
InitialLCD (void)
{
        int 	i;
       
	    //LCD初始命令同步
        for (i=0; i < 4; i++){
            LCD_CMD = 0;			//同步命令
            OptimizationDelay();
        }
		
        WriteLCD(LCD_START_OSC, 1);	//重新啟動LCD 
        mdelay(15);			//延遲等待LCD進入穩定工作狀態
	
        //128 * 128 ; Inervt COM & SEG
        //CMS=1, SGS=1, CN=0, NL3-0=1111:128*128 dots, 
        WriteLCD(LCD_DRV_OUT, 0x030f);		
	
        //B/C=0, EOR=0, NW4-0=0000
        WriteLCD(LCD_WAVEFORM, 0);		
	
        //BS2-0=000:1/11 bias driver, BT1-0=10:Six-times boost
        //DC1-0=11:4-divided clock, AP1-0=11:Large, 
        //SLP=0, Sleep mode off, STB=0:Standby mode off)
        WriteLCD(LCD_PWR_CONTROL, 0x23c);	
	
        //CT5-0=011000:
        WriteLCD(LCD_CONTRAST, 0x18);		
	
        //I/D=1:Increment by 1, AM1-0=00:Horizontal move, LG1=00:Replace mode)
        WriteLCD(LCD_ENTRY_MODE, 0x10);		
	
        //RT2-0=000:No shift)
        WriteLCD(LCD_ROTATION, 0);		
	
        //C=1:Cursor display started, CM1-0=00:White-blink cursor) 
        WriteLCD(LCD_CURSOR, 4);		
	
        //DE6-0=0000000:, DS6-0=0000000:
        WriteLCD(LCD_DB_HEIGHT, 0);		
	
        //SL6-0=0000000:First raster-row displayed at the top
        WriteLCD(LCD_VET_SCROLL, 0);		
	
        //HE6-0=HS6-0=VE6-0=VS6-0=0000000
        WriteLCD(LCD_HOR_CURSOR_POS, 0);		
	
        //VM15-0=0000H:No mask
        WriteLCD(LCD_RAM_MASK, 0);		
	
        //AD10-0=000H
        WriteLCD(LCD_RAM_ADDR, 0);		
	
        //PS1-0=00:Partial scroll off, DHE=0:Double-height display off,
        //GS=0, REV=0, D=1:Dispkay on
        WriteLCD(LCD_DISPLAY, 1);		
	
        LCD_ClearScreen();			//清除畫面成空白	
}
/* ************************************************************************** */



// 規畫LCD操作模式
// 總共多少行, 每行多少字
// 設定游標型態
static void 
Initial_LCD (void)
{
        byFontHeight = 8;                    //文字字型高度為8
        byFontWidth = 8;                     //文字字型寬度為8
        ScrWidth = 128 / byFontWidth;        //LCD每行共多少文字
        ScrHeight = 128 / byFontHeight;      //LCD總共有多少行
        
        InitialLCD();                        //LCD工作模式設定
        
        LCD_CursorMode(BLACK_BLINK_CURSOR);  //黑色閃爍的游標
        LCD_EnableCursor(UM);                //啟動游標        
}
/* ************************************************************************** */



/*********************************** keyboard board **************************/
/*************************************************************************
4X4 KEY PAD
*************************************************************************/
#define KEY_0 		0xdfff
#define KEY_1 		0xfffe
#define KEY_2 		0xfffd
#define KEY_3 		0xfffb
#define KEY_4 		0xffef
#define KEY_5 		0xffdf
#define KEY_6 		0xffbf
#define KEY_7 		0xfeff
#define KEY_8 		0xfdff
#define KEY_9 		0xfbff
#define KEY_A 		0xfff7
#define KEY_B 		0xff7f
#define KEY_C 		0xf7ff
#define KEY_D 		0x7fff
#define KEY_STAR 	0xefff
#define KEY_POND 	0xbfff

static unsigned char Creator_key2num (unsigned short ch);
/* ************************************************************************** */



static void 
Reset_KEYPAD (void)
{
        last_scan_key = 0xffff;
        scan_key_in_idx = 0;
        scan_key_out_idx = 0;
        key_assigned = UM;
}
/* ************************************************************************** */



static void 
Initial_KEYPAD (void)
{
        unsigned char 	i;
		
        Reset_KEYPAD();
        for(i=0; i<16; i++)	
            scan_key_buf[i] = 0xffff;
}
/* ************************************************************************** */



static unsigned char 
KEYPAD_get_key (unsigned char *ch)
{
        unsigned short 	temp;
	
        if (scan_key_out_idx != scan_key_in_idx) {
            temp = scan_key_buf[scan_key_out_idx++];
            scan_key_out_idx &= 0xf;
            *ch = Creator_key2num(temp);
            
            return (OK);
        }
        
        return (UM);
}
/* ************************************************************************** */



static unsigned char 
KEYPAD_chk_key (unsigned char *ch)
{
        unsigned short 	temp;
	
        if (scan_key_out_idx != scan_key_in_idx) {
            temp = scan_key_buf[scan_key_out_idx];
            *ch = Creator_key2num(temp);
            return (OK);
        }
	
        return (UM);
}
/* ************************************************************************** */



static unsigned short key2num_tbl[16] = {
		KEY_0, KEY_1, KEY_2, KEY_3,
		KEY_4, KEY_5, KEY_6, KEY_7,
		KEY_8, KEY_9, KEY_A, KEY_B,
		KEY_C, KEY_D, KEY_STAR, KEY_POND
		};

static  unsigned char KeyASCIICode[] = {
	'0', 	'1',	'2',	'3',	
	'4', 	'5',	'6',	'7',	
	'8', 	'9',	'A',	'B',	
	'C', 	'D',	'*',	'#'
};		
/* ************************************************************************** */



static unsigned char 
Creator_key2num (unsigned short ch)
{
        unsigned char idx;
		
        for (idx=0; idx<16; idx++) {
            if (ch == key2num_tbl[idx]) 
                return (idx);
        }
        
        return (0xff);
}
/* ************************************************************************** */



int
Creator_pxa270_led_cmd (int cmd, unsigned char led)
{
        int		rc = 0;
        unsigned char 	bit ;
	
        switch (cmd){
        case LED_IOCTL_SET : {
            /* 
             * led value : 1 : 亮, 0 : 不亮
             * H/W 	: 0 : 亮, 1 : 不亮
             */        	
            scan_led =  ((~led) << 8);
            break;       
        }		
        case LED_IOCTL_BIT_SET : {
            int i ;
            	
            if (led >= 8)
                return(-EINVAL);
     	
            bit = 1;	
            for (i=0; i < led; i++)
                bit <<= 1;	

            scan_led &=  ((~bit) << 8);
            break;       
        }
        case LED_IOCTL_BIT_CLEAR : {
            int 	i ;
            
            if (led >= 8)
                return(-EINVAL);        
 	    	
            bit = 1;	
            for (i=0; i < led; i++)
                bit <<= 1;	

            scan_led |=  (bit << 8);
            break;   		
        }
        default :
            return(-ENOTTY);	
        }	
        if (rc == 0)
            IO_REG2 = scan_led | 0xfe;	
            
        return (rc);		
}
/* ************************************************************************** */



static void 
lcd_hwsetup (void)
{
        tick_count = 0;
        tick_sec = tick_min = tick_hour = 0;	
}	
/* ************************************************************************** */



static int 
creator_pxa270_lcdtxt_open (struct inode *inode, struct file *filep)
{
        return (0);
}
/* ************************************************************************** */


static int 
creator_pxa270_lcdtxt_release (struct inode *inode, struct file *filep)
{
        return (0);
}
/* ************************************************************************** */



static ssize_t creator_pxa270_lcdtxt_write(struct file *filep, const char *buf, size_t count, loff_t *ppos)
{
        char	*dp, c;
        int	num;

        dp = (char *) buf;

        for (num = 0; (num < count); num++) {
            GET_USER(c, dp++);
            LCD_putchar(c);
        }

        return (num);
}
/* ************************************************************************** */



static struct timer_list scan_timer;
static unsigned char 	byScanning ;

/* -----------------------------------------------------------------------------------
7Segment LED
------------------------------------------------------------------------------------- */
static unsigned char Start7SegmentLED ;

/*************************************************************************
7 SEGMENT LED
*************************************************************************/

static const UC LED_tbl[] = {
		0xc0, 0xf9, 0xa4, 0xb0, 0x99,
		0x92, 0x82, 0xf8, 0x80, 0x90,
		0xa0, 0x83, 0xc6, 0xa1, 0x84,
		0x8e, 0x7f, 0xff
};
/* ************************************************************************** */



static void 
Initial_7SEG (void)
{
        seven_seg_buf[0] = 0x90;
        seven_seg_buf[1] = 0xA3;
        seven_seg_buf[2] = 0xA3;
        seven_seg_buf[3] = 0xA1;		
};
/* ************************************************************************** */



static int 
_7SEG_put_led (_7seg_info_t *info)
{
        int		rc = 0, i;
        unsigned long 	value;
        unsigned char 	Index, which, OneData, SetLED;

        which = info->Which;
        value = info->Value ;	
	
        switch (info->Mode){
        case _7SEG_MODE_PATTERN : {
            for(i=0, Index=3; i < 4; i++, Index--){
                SetLED = which & 1 ;
                which >>= 1;
                if (!SetLED) {
                    continue ;
                }
                OneData = value & 0xff ;
                seven_seg_buf[Index] = (~OneData);	    	    
                value >>= 8 ;		    	
            }		    
	        break ;
        }
        case _7SEG_MODE_HEX_VALUE :{
            for(i=0, Index=3; i < 4; i++, Index--){
                SetLED = which & 1 ;
                which >>= 1;
                if (!SetLED) {
                    continue ;
                }
                OneData = value & 0xf ;
                seven_seg_buf[Index] = LED_tbl[OneData];	    	    
                value >>= 4 ;	    	
            }	
            break ; 	
        }    
        default :		
           return (-ENOTTY);	
       }	
       	
       return (rc);		
}	
/* ************************************************************************** */



static irqreturn_t 
_7segment_timer_irq (int iqr, void *dev_id, struct pt_regs *regs)
{                         
        unsigned long flags;
            
        seven_seg_idx++;
        seven_seg_idx &= 3;               
        
        spin_lock_irqsave(&creator_io.creator_lock, flags);           
        IO_REG0 = creator_io.io_reg0 | tbl_7seg_com[seven_seg_idx] | seven_seg_buf[seven_seg_idx];	                                  
        spin_unlock_irqrestore(&creator_io.creator_lock, flags);         
                    
        return (IRQ_HANDLED);
}	
/* ************************************************************************** */



static int 
_7segment_timer_operation (enum lcd_timer tt)
{
        unsigned long flags;    
        int 		  result=0 ;

        switch (tt){
        case TTIMER_SETUP :{   
            spin_lock_irqsave(&_7segment_time_lock, flags);               
            OMCR4 = (1<<7) + (1<<6) + (0<<4) + (1<<3) + (2);   //  Periodic, Reset OSCR4; 1 ms       
            OSMR4 = TIMER_COUNT ;
          
            OIER &= ~(1<<OIER_E4);   // disable OST4
            OSCR4 = 0;
            spin_unlock_irqrestore(&_7segment_time_lock, flags);     

            result = request_irq(INT_NO, _7segment_timer_irq, SA_INTERRUPT, g_7_segment_timer_id, NULL);                        
            if (result){
                printk(KERN_WARNING "Creaetro :ca'nt get assigned irq%i\n", INT_NO);
                return (result);
            }   
            Start7SegmentLED = OK;             
            break;
        }
		case TTIMER_START: {
            spin_lock_irqsave(&_7segment_time_lock, flags);  		
            OMCR4 = (1<<7) + (1<<6) + (0<<4) + (1<<3) + (2);   //  Periodic, Reset OSCR4; 1 ms       
            OSMR4 = TIMER_COUNT ;
                        
 	        OIER |= (1 << OIER_E4);	/* enable match on timer 4 to cause interrupts */ 
	    	OSCR4 = 0;		/* initialize free-running timer, force first match */  	
            spin_unlock_irqrestore(&_7segment_time_lock, flags); 	    	          
            Start7SegmentLED = OK;               
            break;
        }
        case TTIMER_STOP :{
            spin_lock_irqsave(&_7segment_time_lock, flags);            
            OIER &= ~(1<<OIER_E4);   // disable OST4		    
            OMCR4 = 0;	          
            spin_unlock_irqrestore(&_7segment_time_lock, flags);              

            Start7SegmentLED = UM;		    
            break;
        }    
        }       
        return (result);	
}
/* ************************************************************************** */



int
Creator_pxa270_7Segment_cmd (int cmd, struct _7Seg_Info *Info)
{
        int	rc = 0;
		
        switch(cmd){
        case _7SEG_IOCTL_ON :{
            if (Start7SegmentLED)
                return (rc);
        	
            if (_7segment_timer_operation(TTIMER_START) < 0){
                rc = (-EIO);
            }	    
            break ;
        }
        case _7SEG_IOCTL_OFF :{
            if (Start7SegmentLED)	
                _7segment_timer_operation(TTIMER_STOP);			
            break ;			
        }	    
        case _7SEG_IOCTL_SET :
            return (_7SEG_put_led(Info));	    	
	    
        default : 
            return (-ENOTTY);			    
        }	
        
        return (rc);
}
/* ************************************************************************** */



/*************************************************************************
EEPROM
*************************************************************************/
#define	EE_CMD_READ		0x30000
#define	EE_CMD_EWEN		0x26000
#define	EE_CMD_ERASE	0x38000
#define	EE_CMD_WRITE	0x28000
#define	EE_CMD_EWDS		0x20000

#define	EE_DELAY	    Delay(10);


static unsigned char 
EE_ISSUE_CMD (unsigned long command)
{
		unsigned long  mask, idx;
		unsigned short d_out;
		
		
		creator_io.io_reg0 |= 0x1000;		//EE_CS = 1
		EE_DELAY
		creator_io.io_reg0 &= 0xdfff;		//EE_CK = 0
		EE_DELAY
		
		for(idx=0, mask = 0x20000; idx < 18; idx++, mask>>=1) {
			if((mask & command) == 0) 	creator_io.io_reg0 &= 0xbfff;
			else						creator_io.io_reg0 |= 0x4000;
			EE_DELAY
		
			creator_io.io_reg0 |= 0x2000;	//EE_CK = 1
			EE_DELAY
			
			d_out = (d_out << 1) | (CPLD_STATUS & 1);	//get EE_DO

			creator_io.io_reg0 &= 0xdfff;	//EE_CK = 0
			EE_DELAY
		}
		creator_io.io_reg0 &= 0xcfff;		//EE_CS = EE_CK = 0
		EE_DELAY
		
		Delay(5);	//delay 5ms for write operation (for any command)
		return((unsigned char)(d_out));
}
/* ************************************************************************** */



static void 
EE_WRITE (unsigned char addr, unsigned char data)
{
		unsigned long  addr1, data1;

		addr1 = ((unsigned long)(addr));
		data1 = ((unsigned long)(data));
		
		EE_ISSUE_CMD(EE_CMD_EWEN | 0 | 0);
		EE_ISSUE_CMD(EE_CMD_WRITE | (addr1<<8) | data1);
		EE_ISSUE_CMD(EE_CMD_EWDS | 0 | 0);
}
/* ************************************************************************** */



static unsigned char 
EE_READ (unsigned char addr)
{
		unsigned long addr1;

		addr1 = ((unsigned long)(addr));
		return (EE_ISSUE_CMD(EE_CMD_READ | (addr1<<8) | 0));
}
/* ************************************************************************** */



static void 
ScanTimer (void)
{
        UN_CVT cvt;	
        unsigned long flags;

        del_timer(&scan_timer);
        byScanning = UM ;

        if (1) {
            if((scan_key_in_idx+1) == scan_key_out_idx) {	//Key_Pad Overfllow
	    	//TODO
            }
            else {
                IO_REG2 = scan_led | 0xfe;
                cvt.b[0] = (UC)((IO_REG1 & 0x0f00) >> 8);
                IO_REG2 = scan_led | 0xfd;
                cvt.b[0] |= (UC)((IO_REG1 & 0x0f00) >> 4);
                IO_REG2 = scan_led | 0xfb;
                cvt.b[1] = (UC)((IO_REG1 & 0x0f00) >> 8);
                IO_REG2 = scan_led | 0xf7;
                cvt.b[1] |= (UC)((IO_REG1 & 0x0f00) >> 4);
                if (last_scan_key == cvt.w[0]) {
                    if (key_assigned == UM) {
                        if (cvt.w[0] != 0xffff) {
                            scan_key_buf[scan_key_in_idx++] = cvt.w[0];
                            scan_key_in_idx &= 0xf;
                            wake_up_interruptible(&wq_read);
                        }
                        key_assigned = OK;
                    }
                }
                else {
                    key_assigned = UM;
                }
                last_scan_key = cvt.w[0];
	        }
	    }	
	    if (++tick_count == KEYPAD_SCAN_FREQ){
            tick_count = 0;
            spin_lock_irqsave(&time_tick_lock, flags);
            tick_sec++;
            if (tick_sec >= 60) {
                tick_sec = 0;
                tick_min++;
                if (tick_min >= 60) {
                    tick_min = 0;
                    tick_hour++;
                }        
            }	 
            spin_unlock_irqrestore(&time_tick_lock, flags);                   
        }         

        scan_timer.expires =  jiffies + KEYPAD_SCAN_PERIOD;		
        add_timer(&scan_timer);
        byScanning = TRUE ;
}
/* ************************************************************************** */



int 
creator_pxa270_lcdtxt_ioctl (struct inode *inode, struct file *filep, unsigned int command, unsigned long argAddress)
{
        lcd_write_info_t	display;	
	    void                *arg;	    
	    int                 arg_size ;
	    int                 dir;        
	    int                 rc = 0;	
       
        /*
         * 分離type如果遇到錯誤的cmd, 就直接傳回ENOTTY
         */
        if (_IOC_TYPE(command) != LCD_IOCTL_MAGIC) return (-ENOTTY);
	
        arg = (void*)argAddress;        
	    arg_size = _IOC_SIZE(command);
	    dir = _IOC_DIR(command);		
        
        switch (command) {
        case LCD_IOCTL_CLEAR :{		 /* 清除整個螢幕 */
            LCD_ClearScreen();
            break;						
        }	    
        case LCD_IOCTL_WRITE : {
            int	index;	
	    
            if (copy_from_user(&display, arg, arg_size))
                return (-EINVAL);
            for (index = 0; index < (display.Count); index++){
                LCD_putchar(display.Msg[index]);
            }			  	
            break ;		    
        }
        case LCD_IOCTL_CUR_ON :
            LCD_EnableCursor(OK);	
            break ;	
        case LCD_IOCTL_CUR_OFF :
            LCD_EnableCursor(OFF);		
            break ;		    
        case LCD_IOCTL_CUR_GET : {
            if (copy_from_user(&display, arg, arg_size))
                return (-EINVAL);	
            LCD_GetCursor(&display.CursorX, &display.CursorY);	
            if (copy_to_user(arg, &display, arg_size))	    	
                return (-EINVAL);	  
            break ;
        }	    
        case LCD_IOCTL_CUR_SET :{
            if (copy_from_user(&display, arg, arg_size))   
                return (-EINVAL);
            LCD_SetCursor(display.CursorX, display.CursorY);			  	
            break ;
        }	    
        case LCD_IOCTL_DRAW_FULL_IMAGE :{
            lcd_full_image_info_t image;	
            if (copy_from_user(&image, arg, arg_size))   
                return (-EINVAL);	
            LCD_DrawFullImage(image.data);    
            break;	    
        }	    	    
        case LED_IOCTL_SET : 
        case LED_IOCTL_BIT_SET : 
        case LED_IOCTL_BIT_CLEAR : {
            unsigned short led;
	    	
            if (copy_from_user(&led, arg, arg_size))
                return (-EINVAL);

            if (Creator_pxa270_led_cmd(command, led) < -EFAULT)
                return (-EINVAL);
            break; 
        }   
        case DIPSW_IOCTL_GET :{
            unsigned short DIPSWs ;
	    	
            DIPSWs = (UC)(IO_REG1); 
            if (copy_to_user(arg, &DIPSWs, arg_size))
                return (-EINVAL);
            break;	
        }	    
        case KEY_IOCTL_GET_CHAR : {
            unsigned short data ;		
            unsigned char key ;
	    
            if (KEYPAD_get_key(&key)){
                data = (key << 8) + KeyASCIICode[key] ;
                if (copy_to_user(arg, &data, arg_size))	    	
                    return (-EINVAL);
                return (0);
            }		    
            return (-EIO);
        }	    
        case KEY_IOCTL_WAIT_CHAR : {
            unsigned short data ;		
            unsigned char key ;
	    		
            if (!KEYPAD_chk_key(&key))
                interruptible_sleep_on(&wq_read);
	    
            if (KEYPAD_get_key(&key)){
                data = (key << 8) + KeyASCIICode[key] ;
                if (copy_to_user(arg, &data, arg_size))	    	
                    return (-EINVAL);
                return (0);
            }		    
	        return (-EIO);		
        }
        case KEY_IOCTL_CANCEL_WAIT_CHAR : {
            wake_up_interruptible(&wq_read);		
            return (0);		
        }		
		
        case KEY_IOCTL_CHECK_EMTPY : {
            unsigned short ascii;	
            unsigned char key ;
	    	
            if (KEYPAD_chk_key(&key)){
                ascii = (key << 8) + KeyASCIICode[key];
                if (copy_to_user(arg, &ascii, arg_size))	
                return (-EINVAL);
	    	
                return (0);
            }     		    
            return (-EINVAL);    	
        }
	
        case KEY_IOCTL_CLEAR : 
            Reset_KEYPAD();
            break;
	    	
        case _7SEG_IOCTL_ON :
        case _7SEG_IOCTL_OFF :	
            return (Creator_pxa270_7Segment_cmd(command, NULL));
            
        case _7SEG_IOCTL_SET :{
            _7seg_info_t  data;
   
            if (copy_from_user(&data, arg, arg_size)){
                return (-EINVAL);
            }	
            return (Creator_pxa270_7Segment_cmd(command, &data));
        }
	
        case _TIME_IOCTL_GET :{
	        _Timer_info_t time;
            unsigned long flags;
	    	    
            spin_lock_irqsave(&time_tick_lock, flags);	    	    
            time.tick_sec = tick_sec;
            time.tick_min = tick_min ;
            time.tick_hour = tick_hour ;	  
            spin_unlock_irqrestore(&time_tick_lock, flags);
	    	    
            if (copy_to_user(arg, &time, arg_size))	    	
                return (-EINVAL);
            break;  
        }    
        case _TIME_IOCTL_SET :{	
            _Timer_info_t time;
            unsigned long flags;
	    
            if (copy_from_user(&time, arg, arg_size))	    	
                return (-EINVAL);			    

            spin_lock_irqsave(&time_tick_lock, flags);
            tick_sec = time.tick_sec;
            tick_min = time.tick_min;
            tick_hour = time.tick_hour ;	  
            spin_unlock_irqrestore(&time_tick_lock, flags);	    
   
            break ;		    
        }    
        case _EEPROM_IOCTL_SET :{
            _EEPROM_info_t info ;            
            if (copy_from_user(&info, arg, arg_size))	    	
                return (-EINVAL);                   
                         
            EE_WRITE(info.addr, info.data);
            break ;
        }    
        case _EEPROM_IOCTL_GET :{
            _EEPROM_info_t info ;            
            if (copy_from_user(&info, arg, arg_size))	    	
                return (-EINVAL);        
                
            info.data = EE_READ(info.addr); 
            if (copy_to_user(arg, &info, arg_size))	    	
                return (-EINVAL);                           
                
            break ;
        }    
        default:
            rc = -ENOTTY;
            break;
        }

        return (rc);
}
/* ************************************************************************** */



/*
 *	Exported file operations structure for driver...
 */
struct file_operations	creator_pxa270_lcdtxt_fops = 
{
    owner:      THIS_MODULE,
	write:		creator_pxa270_lcdtxt_write,
	ioctl:		creator_pxa270_lcdtxt_ioctl,
	open:		creator_pxa270_lcdtxt_open,
	release:    creator_pxa270_lcdtxt_release,
};


static struct cdev lcd_cdev = {
	.kobj	=	{.name = MODULE_NAME, },
	.owner	=	THIS_MODULE,
};
/* ************************************************************************** */



static int __init 
init_module_drv_lcdtxt (void)
{
        struct  cdev   *pcdev;    
        dev_t   devno;    
        int	    error;
      
        spin_lock_init(&_7segment_time_lock);
        spin_lock_init(&time_tick_lock);        

	    init_waitqueue_head(&wq_read);             
          
          
	    devno = MKDEV(MAJOR_NUM, 0);	    
        if (register_chrdev_region(devno, MAX_MINORS, MODULE_NAME)){    
            printk(KERN_ALERT "%s: can't get major %d\n", MODULE_NAME, MAJOR_NUM);			    
            return (-EBUSY);
        }		    
        pcdev = &lcd_cdev;		    
    	cdev_init(pcdev, &creator_pxa270_lcdtxt_fops);
	    pcdev->owner = THIS_MODULE;
    
    	
        /* Register lcdtxt as character device */
        error = cdev_add(pcdev, devno, MAX_MINORS);
        if (error) {
            kobject_put(&pcdev->kobj);
            unregister_chrdev_region(devno, MAX_MINORS);

            printk(KERN_ERR "error register %s device\n", MODULE_NAME);
            
            return (-EBUSY);
        }        
        printk(KERN_ALERT "%s: Version : %s %s\n", MODULE_NAME, MODULE_VERSION_INFO, COPYRIGHT);           

        /* Hardware specific initialization */
        lcd_hwsetup();
       

        Initial_LCD();             
        LCD_EnableCursor(OK);
	
        Initial_KEYPAD();
        Initial_7SEG();	        
          
        /*
         * initial timer for scanning 7-segment
         */ 
        
        if (_7segment_timer_operation(TTIMER_SETUP))
            printk(KERN_NOTICE "  !!!!! 7-Segment LEDS TIMER SETUP FAILED   \n");       
                  
                  
        init_timer(&scan_timer);
        byScanning = TRUE ;
        
        /*
         * initial S/W timer for scanning keypad
         */
        scan_timer.expires =  jiffies + KEYPAD_SCAN_PERIOD;		
        scan_timer.function = (void *)ScanTimer;	       
        add_timer(&scan_timer);	          
	
        //Start7SegmentLED = UM ;	             
       
        return (0);  
}
/* ************************************************************************** */



static void __exit 
cleanup_module_drv_lcdtxt (void)
{	       
        /*
         * close timer for scaning 7-segment
         */
        _7segment_timer_operation(TTIMER_STOP);
        disable_irq(INT_NO);	       
        free_irq(INT_NO, NULL);     
       
        if (byScanning)
            del_timer(&scan_timer);	   

        wake_up_interruptible(&wq_read);
        cdev_del(&lcd_cdev);
       	unregister_chrdev_region(MKDEV(MAJOR_NUM, 0), MAX_MINORS);
        printk(KERN_ALERT "%s: removed\n", MODULE_NAME);	
}	
/* ************************************************************************** */



/* here are the compiler macro for module operation */
module_init(init_module_drv_lcdtxt);
module_exit(cleanup_module_drv_lcdtxt);

MODULE_AUTHOR(MODULE_AUTHOR_STRING);
MODULE_DESCRIPTION(MODULE_DESCRIPTION_STRING);


/*****************************************************************************/
