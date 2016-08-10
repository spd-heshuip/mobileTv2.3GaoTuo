
#ifndef __RTL2832U_FE_H__
#define __RTL2832U_FE_H__


#include "rtl2832u_io.h"

//#include <linux/jiffies.h>

#ifndef u32 
#define u32 unsigned long int 
#endif
#ifndef u16
#define u16 unsigned int 
#endif

#define  UPDATE_FUNC_ENABLE_2840      0
#define  UPDATE_FUNC_ENABLE_2836      1
#define  UPDATE_FUNC_ENABLE_2832      1

#define UPDATE_PROCEDURE_PERIOD_2836	       (HZ/5) //200ms = jiffies*1000/HZ
#define UPDATE_PROCEDURE_PERIOD_2832       (HZ/5)  //200ms

typedef enum{
	RTK_UNKNOWN = 0,
	RTK_VIDEO,
	RTK_AUDIO,
}RTL2832_WORK_TYPE;

struct rtl2832_reg_addr{
	RegType			reg_type;
	unsigned short	reg_addr;
	int				bit_low;
	int				bit_high;
};



typedef enum{
	RTD2831_RMAP_INDEX_USB_CTRL_BIT5 =0,
	RTD2831_RMAP_INDEX_USB_STAT,		
	RTD2831_RMAP_INDEX_USB_EPA_CTL,
	RTD2831_RMAP_INDEX_USB_SYSCTL,
	RTD2831_RMAP_INDEX_USB_EPA_CFG,
	RTD2831_RMAP_INDEX_USB_EPA_MAXPKT,
	RTD2831_RMAP_INDEX_USB_EPA_FIFO_CFG,	

	RTD2831_RMAP_INDEX_SYS_DEMOD_CTL,
	RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_VAL,		
	RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT3,		
	RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT3,			
	RTD2831_RMAP_INDEX_SYS_GPIO_CFG0_BIT67,
	RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1,
	RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT1,		
	RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT1,	
	RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT6,		
	RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT6,
	RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT5,
	RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT5,
	RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT0,
	RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT0,
	RTD2831_RMAP_INDEX_SYS_GPIO_CFG0_BIT01,
	RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT4,
	RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT4,
	RTD2831_RMAP_INDEX_SYS_GPIO_CFG1_BIT01,
#if 0	
    RTD2831_RMAP_INDEX_SYS_GPD,
    RTD2831_RMAP_INDEX_SYS_GPOE,
    RTD2831_RMAP_INDEX_SYS_GPO,
    RTD2831_RMAP_INDEX_SYS_SYS_0,    
#endif 

} rtl2832_reg_map_index;



#define USB_SYSCTL				0x0000 	
#define USB_CTRL				0x0010
#define USB_STAT				0x0014	
#define USB_EPA_CTL				0x0148  	
#define USB_EPA_CFG				0x0144
#define USB_EPA_MAXPKT			0x0158  
#define USB_EPA_FIFO_CFG		0x0160 

#define DEMOD_CTL				0x0000	
#define GPIO_OUTPUT_VAL		0x0001
#define GPIO_OUTPUT_EN			0x0003
#define GPIO_DIR					0x0004
#define GPIO_CFG0				0x0007
#define GPIO_CFG1				0x0008	
#define DEMOD_CTL1				0x000b






#define BIT0		0x00000001
#define BIT1		0x00000002
#define BIT2		0x00000004
#define BIT3		0x00000008
#define BIT4		0x00000010
#define BIT5		0x00000020
#define BIT6		0x00000040
#define BIT7		0x00000080
#define BIT8		0x00000100
#define BIT9		0x00000200
#define BIT10	0x00000400
#define BIT11	0x00000800
#define BIT12	0x00001000
#define BIT13	0x00002000
#define BIT14	0x00004000
#define BIT15	0x00008000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000


/* DTMB related 

typedef enum {
	PAGE_0 = 0,
	PAGE_1 = 1,
	PAGE_2 = 2,
	PAGE_3 = 3,
	PAGE_4 = 4,	
	PAGE_5 = 5,	
	PAGE_6 = 6,	
	PAGE_7 = 7,	
	PAGE_8 = 8,	
	PAGE_9 = 9,	
};*/


#define	SUPPORT_DVBT_MODE	0x01
#define	SUPPORT_DTMB_MODE	0x02
#define	SUPPORT_DVBC_MODE	0x04

#define INPUT_ADC_LEVEL 	-8
typedef enum {
	RTL2832 = 0,
	RTL2836,
	RTL2840	
}DEMOD_TYPE;

typedef enum _PID_FILTER_MODE
{
	PID_FILTER_PASS=0,		 // only set PIDs pass through
	PID_FILTER_ENABLE,		 // only set PIDs can't pass through	
	PID_FILTER_DISABLE		 // Disable PID, let all TS pass through
}PID_FILTER_MODE;


int  rtl2832_hw_reset();




int rtl2832_read_signal_quality(
	u32*	quality);
int 
rtl2832_read_signal_strength(
	u16*	strength);
#endif // __RTD2830_PRIV_H__


