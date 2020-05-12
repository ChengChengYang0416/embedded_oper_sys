// --------------------------------------------------------------------
//	Directory  : linux/arch/arm/mach-creator-s3c2410/
//
//   Title     :  creator-s3c2410-core.c
//             :
//   Library   :
//             :
//   Developers:  MICROTIME MDS group
//             :
//   Purpose   :  Global variables
//             :
//   Limitation:
//             :
//   Note      :
//             :
// --------------------------------------------------------------------
//   modification history :
// --------------------------------------------------------------------
//   Version | mod. date: |
//   Vx.xx   | mm/dd/yyyy | 
//   V1.00   | 05/22/2006 | First release
//   V1.01   | 05/08/2009 | support linux 2.6.27
//   V1.02   | 12/16/2009 | fixed: check Creator Board exist
//   V1.03   | 12/28/2009 | fixed: check Creator Board version <= 0x11
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
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/mach-types.h>
#include "asm/arch/lib/creator_pxa270_core.h"


/*************************************************************************
Constant define
*************************************************************************/

/* ****** Debug Information ******************************************** */
//#define DEBUG
#ifdef DEBUG 
#define MSG(string, args...) printk("<1>" string, ##args)
#else   
#define MSG(string, args...)
#endif



/*************************************************************************
Function prototypes
*************************************************************************/


/*************************************************************************
Variable define
*************************************************************************/
creator_io_t creator_io ;
/* ************************************************************************** */


void 
creator_cf_reset (void)
{ 
        unsigned long flags;
            
        spin_lock_irqsave(&creator_io.creator_lock, flags);            
        creator_io.io_reg0 &= 0x7FFFF;		// Bit 15 : CF_nRST = 0
        IO_REG0 = creator_io.io_reg0;
        spin_unlock_irqrestore(&creator_io.creator_lock, flags);        
        
        
        mdelay(10);
        
        spin_lock_irqsave(&creator_io.creator_lock, flags);                 
        creator_io.io_reg0 |= 0x8000;		// Bit 15 : CF_nRST = 1
        IO_REG0 = creator_io.io_reg0;        
        spin_unlock_irqrestore(&creator_io.creator_lock, flags);        
        
        mdelay(500);       
}
/* ************************************************************************** */



int 
creator_pxa270_core_init (void)
{  
        unsigned long flags;
            
        spin_lock_init(&creator_io.creator_lock);    
        
        spin_lock_irqsave(&creator_io.creator_lock, flags);        
        creator_io.io_reg0 = 0xc000;
        IO_REG0 = creator_io.io_reg0;
        spin_unlock_irqrestore(&creator_io.creator_lock, flags);
        
        mdelay(1);

        spin_lock_irqsave(&creator_io.creator_lock, flags);  
        creator_io.cpld_ctrl = 0x3D;
        CPLD_CTRL = creator_io.cpld_ctrl;
        spin_unlock_irqrestore(&creator_io.creator_lock, flags);       

        creator_io.cf_reset = creator_cf_reset;
        creator_io.cf_reset();     
        return 0;

}
/* ************************************************************************** */
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



#define SET_ADDRESS_LCD(x)		WriteLCD(LCD_RAM_ADDR, x);
#define SET_RAM_DATA_LCD(x)		WriteLCD(LCD_RAM_DATA, x);
#define LCD_COMMAND(command)	{LCD_CMD=0xff; LCD_CMD=(UI)command;}

static void
OptimizationDelay (void)
{
}	





/*Procedure*******************************
*          Create date : 03/04/2003       *
*          Modify date :                  *
*                                         *
******************************************/
unsigned short 
ReadLCD (unsigned short addr)
{
	unsigned short data;

	LCD_CMD = 0xff;	//MSB
	OptimizationDelay();	// for Optimization	
	LCD_CMD = addr;	//LSB
	data = LCD_DATA << 8;
	data |= LCD_DATA;

	if (addr == LCD_RAM_DATA) {
	    LCD_CMD = 0xff;	//MSB
    	OptimizationDelay();	// for Optimization	    
	    LCD_CMD = addr;	//LSB
	    data = LCD_DATA << 8;
	    data |= LCD_DATA;
	}
	return (data);
}



/*Procedure*******************************
*          Create date : 03/04/2003       *
*          Modify date :                  *
*                                         *
******************************************/
void 
WriteLCD (UI addr, UI data)
{
	LCD_CMD = 0xff;	//MSB
	OptimizationDelay();	// for Optimization	
	LCD_CMD = addr;	//LSB
		
	LCD_DATA = (data >> 8) ;
	OptimizationDelay();	// for Optimization	
	LCD_DATA = data;
}



static void 
InitialLCD (void)
{
	int 	i;
	
	// The eight upper and lower bits can be corrected by a reset triggered by
	// consecutively writing a 00H instruction four times.
	//		
	for (i=0; i < 4; i++){
            LCD_CMD = 0;
            OptimizationDelay();
        }
		
	//The start oscillation instruction restarts the oscillator from the halt state in the standby mode. After issuing
	//this instruction, wait at least 10 ms for oscillation to stabilize before issuing the next instruction	
	//
	WriteLCD(LCD_START_OSC, 1);
	mdelay(15);
	
	WriteLCD(LCD_DRV_OUT, 0x030f);		// 128 * 128 ; Inervt Comm&Segment
	WriteLCD(LCD_WAVEFORM, 0);
	WriteLCD(LCD_PWR_CONTROL, 0x23c);	// 0x033c
	WriteLCD(LCD_CONTRAST, 0x18);
	WriteLCD(LCD_ENTRY_MODE, 0x10);
	WriteLCD(LCD_ROTATION, 0);
	
	WriteLCD(LCD_CURSOR, 4);
	WriteLCD(LCD_DB_HEIGHT, 0);
	WriteLCD(LCD_VET_SCROLL, 0);
	WriteLCD(LCD_HOR_CURSOR_POS, 0);
	WriteLCD(LCD_RAM_MASK, 0);
	WriteLCD(LCD_RAM_ADDR, 0);	
	//WriteLCD(LCD_DISPLAY, 1); 
	//LCD_ClearScreen();	
}



/*Procedure*******************************
*          Create date : 03/17/2003       *
*          Modify date :                  *
*                                         *
******************************************/
void LCD_PutRAMData (UI address, UI data)
{
	    SET_ADDRESS_LCD(address);
	    SET_RAM_DATA_LCD(data);
}



/*Procedure*******************************
*          Create date : 03/17/2003       *
*          Modify date :                  *
*                                         *
******************************************/
UI LCD_GetRAMData (UI address)
{
	    SET_ADDRESS_LCD(address);
	    return(ReadLCD(LCD_RAM_DATA));
}



/**
 * Purpose ............: check Creator Board present ?
 * @param  None  ......: 
 * @return     ........: return >=0 when Creator Board present, otherwise return -1;
 */ 
static int
check_creator_board_exist (void)
{
	    int i;
	    unsigned short addr, lcd_data ;
	    unsigned char  CreatorBoardVersion ;       	
	    
	    /*
	     *  Set main board CPLD : mask creator board IRQ
	     *  CPLD Version must above 1.2(include)
	     */
        MASTER_INTMASK1 |= (0xF0);        
        
        /*
         * check creator board present.
         */         
        IRQ3_MASK = 0;
        IRQ3_PEND = 3 ;
        if ((IRQ3_MASK & 3) != 0){
        	goto err;
        }    

        IRQ3_MASK = 1;
        IRQ3_PEND = 2 ;
        if ((IRQ3_MASK & 3) != 1){        	
        	goto err;   
        }      

        IRQ3_MASK = 2;
        IRQ3_PEND = 1 ;
        if ((IRQ3_MASK & 3) != 2){  
        	goto err;       
        }    
            
        IRQ3_MASK = 3;
        IRQ3_PEND = 0 ;
        if ((IRQ3_MASK & 3) != 3){
        	goto err;       
        }      
	    /*
	     *  Set main board CPLD : mask creator board IRQ
	     */
        MASTER_INTMASK1 &= (~0xF0);
            
        return (0);
        
err:
        /*
         * maybe CPLD version <= 0x11
         */
      	printk("Check LCD RAM when Creator board version <= 0x11\n");          
        for (i=0; i < 3; i++){
            IRQ3_PEND = 3 ;
	        CreatorBoardVersion = (unsigned char)(CPLD_STATUS >> 8); 	        
	        if (CreatorBoardVersion != 0x10 && CreatorBoardVersion != 0x11){
      	        printk("Warning: Not support Creator board version=%x.\n", CreatorBoardVersion);  	        	
	            return (-1);
	       	}
	    }
	    
	    /* 
	     * double check LCD RAM.
	     */
	    InitialLCD();
        addr = 0x800;
        LCD_PutRAMData(addr, ~addr);        

        mdelay(1);
        lcd_data = LCD_GetRAMData(addr);	    
        if (lcd_data != (unsigned short)(~addr)) {
      	    printk("Warning(%s) : Creator board not exist.\n", __FUNCTION__);          	
        	return (-1);
        }    
	    
        return (0);
}	



/**
 * Purpose ............: Get Creator Board Version.
 * @param  None  ......: 
 * @return     ........: return CPLD Version when Creator Board present, otherwise return 0xFF;
 */ 
unsigned char
creator_GetCreatorCPLDVersion (void)
{      
	    static int first = 1;
	    static unsigned char CreatorBoardVersion ;
	   
	    if (first){
	        if (check_creator_board_exist() >= 0){
	       	    CreatorBoardVersion = (unsigned char)(CPLD_STATUS >> 8);
	        }
	        else{
	            CreatorBoardVersion = CREATOR_BOARD_NOT_EXIST;
	        }
	        first = 0;
	    }	
        return (CreatorBoardVersion);                                 
}    
/* ************************************************************************** */


EXPORT_SYMBOL(creator_io);
EXPORT_SYMBOL(creator_pxa270_core_init);
EXPORT_SYMBOL(creator_cf_reset);
EXPORT_SYMBOL(creator_GetCreatorCPLDVersion);

__initcall(creator_pxa270_core_init);
