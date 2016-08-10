
#include "rtl2832u_fe.h"
#include "rtl2832u_io.h"
#include "demod_rtl2832.h"
//#include <windows.h>

#define dvb_use_rtl2832u_card_type 1
extern int rtl2832_Initialize( void );
extern int rtl2832_SoftwareReset(void);
extern int Mxl603SetFreq_BW(unsigned long int Freq,unsigned int Bw);
extern int Mxl603Initialize(void);
extern int SetRegBitsWithPage(int nRegBitName,unsigned long WritingValue);
extern int rtl2832_SetBandwidthMode(DVBT_BANDWIDTH_MODE BandwidthMode);
extern int rtl2832_SetIfFreqHz(unsigned long IfFreqHz);
extern int rtl2832_func1_ResetReg();
extern void rtl2832_InitRegTable();
int demod_init_setting();
int SetTsExt(void);

static struct rtl2832_reg_addr rtl2832_reg_map[]= {
	/* RTD2831_RMAP_INDEX_USB_CTRL_BIT5*/			{ RTD2832U_USB, USB_CTRL, 5, 5		},
	/* RTD2831_RMAP_INDEX_USB_STAT*/				{ RTD2832U_USB, USB_STAT, 0, 7		},
	/* RTD2831_RMAP_INDEX_USB_EPA_CTL*/			{ RTD2832U_USB, USB_EPA_CTL, 0, 31	},
	/* RTD2831_RMAP_INDEX_USB_SYSCTL*/				{ RTD2832U_USB, USB_SYSCTL, 0, 31		},
	/* RTD2831_RMAP_INDEX_USB_EPA_CFG*/			{ RTD2832U_USB, USB_EPA_CFG, 0, 31	},
	/* RTD2831_RMAP_INDEX_USB_EPA_MAXPKT*/		{ RTD2832U_USB, USB_EPA_MAXPKT, 0, 31},
	/* RTD2831_RMAP_INDEX_USB_EPA_FIFO_CFG*/		{ RTD2832U_USB, USB_EPA_FIFO_CFG, 0, 31},

	/* RTD2831_RMAP_INDEX_SYS_DEMOD_CTL*/			{ RTD2832U_SYS, DEMOD_CTL, 0, 7	       },
	/* RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_VAL*/	{ RTD2832U_SYS, GPIO_OUTPUT_VAL, 0, 7	},
	/* RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT3*/{ RTD2832U_SYS, GPIO_OUTPUT_EN, 3, 3	},
	/* RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT3*/		{ RTD2832U_SYS, GPIO_DIR, 3, 3		},
	/* RTD2831_RMAP_INDEX_SYS_GPIO_CFG0_BIT67*/	{ RTD2832U_SYS, GPIO_CFG0, 6, 7		},
	/* RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1*/		{ RTD2832U_SYS, DEMOD_CTL1, 0, 7	       },	
	/* RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT1*/{ RTD2832U_SYS, GPIO_OUTPUT_EN, 1, 1	},
	/* RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT1*/		{ RTD2832U_SYS, GPIO_DIR, 1, 1		},
	/* RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT6*/{ RTD2832U_SYS, GPIO_OUTPUT_EN, 6, 6	},	
	/* RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT6*/		{ RTD2832U_SYS, GPIO_DIR, 6, 6		},
	/* RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT5*/{ RTD2832U_SYS, GPIO_OUTPUT_EN, 5, 5},
	/* RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT5*/      { RTD2832U_SYS, GPIO_DIR, 5, 5},
	/* RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT5*/{ RTD2832U_SYS, GPIO_OUTPUT_EN, 0, 0},
	/* RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT5*/      { RTD2832U_SYS, GPIO_DIR, 0, 0},
	/* RTD2831_RMAP_INDEX_SYS_GPIO_CFG0_BIT01*/	{ RTD2832U_SYS, GPIO_CFG0, 0, 1},
	/* RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT4*/{ RTD2832U_SYS, GPIO_OUTPUT_EN, 4, 4},
	/* RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT4*/      { RTD2832U_SYS, GPIO_DIR, 4, 4},
	/* RTD2831_RMAP_INDEX_SYS_GPIO_CFG1_BIT01*/	{ RTD2832U_SYS, GPIO_CFG1, 0, 1},

#if 0
	/* RTD2831_RMAP_INDEX_SYS_GPD*/			{ RTD2832U_SYS, GPD, 0, 7		},
	/* RTD2831_RMAP_INDEX_SYS_GPOE*/			{ RTD2832U_SYS, GPOE, 0, 7	},
	/* RTD2831_RMAP_INDEX_SYS_GPO*/			{ RTD2832U_SYS, GPO, 0, 7		},
	/* RTD2831_RMAP_INDEX_SYS_SYS_0*/			{ RTD2832U_SYS, SYS_0, 0, 7	},
#endif

	/* DTMB related */

   
};                                          

static int rtl2832_reg_mask[32]= {
    0x00000001,
    0x00000003,
    0x00000007,
    0x0000000f,
    0x0000001f,
    0x0000003f,
    0x0000007f,
    0x000000ff,
    0x000001ff,
    0x000003ff,
    0x000007ff,
    0x00000fff,
    0x00001fff,
    0x00003fff,
    0x00007fff,
    0x0000ffff,
    0x0001ffff,
    0x0003ffff,
    0x0007ffff,
    0x000fffff,
    0x001fffff,
    0x003fffff,
    0x007fffff,
    0x00ffffff,
    0x01ffffff,
    0x03ffffff,
    0x07ffffff,
    0x0fffffff,
    0x1fffffff,
    0x3fffffff,
    0x7fffffff,
    0xffffffff
};


static void custom_wait_ms(unsigned long WaitTimeMs)
{
	platform_wait(WaitTimeMs);
	return;	
}


static int
custom_i2c_read(
	unsigned char				DeviceAddr,
	unsigned char*			pReadingBytes,
	unsigned long				ByteNum
	)
{
	if ( read_rtl2832_stdi2c(DeviceAddr , pReadingBytes , ByteNum ) ) goto error;
	
	return 0;
error:
	return 1;
}



static int
custom_i2c_write(
	unsigned char				DeviceAddr,
	const unsigned char*			pWritingBytes,
	unsigned long				ByteNum)
{
	if ( write_rtl2832_stdi2c( DeviceAddr , (unsigned char*)pWritingBytes , ByteNum ) ) goto error;
	
	return 0;
error:
	return 1;
}



static int
read_usb_sys_register(
	rtl2832_reg_map_index		reg_map_index,
	int*						p_val)
{
	RegType			reg_type=	rtl2832_reg_map[reg_map_index].reg_type;
	unsigned short	reg_addr=	rtl2832_reg_map[reg_map_index].reg_addr;
	int				bit_low=	rtl2832_reg_map[reg_map_index].bit_low;
	int				bit_high=	rtl2832_reg_map[reg_map_index].bit_high;

	int	n_byte_read=(bit_high>> 3)+ 1;

	*p_val= 0;
	if (read_usb_sys_int_bytes(reg_type, reg_addr, n_byte_read, p_val)) goto error;

	*p_val= ((*p_val>> bit_low) & rtl2832_reg_mask[bit_high- bit_low]);
 
	return 0;

error:
	return 1;
}




static int
write_usb_sys_register(
	rtl2832_reg_map_index		reg_map_index,
	int						val_write)
{
	RegType			reg_type=	rtl2832_reg_map[reg_map_index].reg_type;
	unsigned short	reg_addr=	rtl2832_reg_map[reg_map_index].reg_addr;
	int				bit_low=	rtl2832_reg_map[reg_map_index].bit_low;
	int				bit_high=	rtl2832_reg_map[reg_map_index].bit_high;
	
	int	n_byte_write=	(bit_high>> 3)+ 1;
	int	val_read= 0;
	int	new_val_write;

	if (read_usb_sys_int_bytes( reg_type, reg_addr, n_byte_write, &val_read)) goto error;

	new_val_write= (val_read & (~(rtl2832_reg_mask[bit_high- bit_low]<< bit_low))) | (val_write<< bit_low);

	if (write_usb_sys_int_bytes( reg_type, reg_addr, n_byte_write, new_val_write)) goto error;
	return 0;
	
error:
	return 1;
}



static int set_demod_power(unsigned char onoff)
{
	int			data;
	
	if ( read_usb_sys_register(RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_VAL, &data) ) goto error;		
	if(onoff)		data &= ~(BIT0);   //set bit0 to 0
	else			data |= BIT0;		//set bit0 to 1	
	data &= ~(BIT0);   //3 Demod Power always ON => hw issue.	
	if ( write_usb_sys_register(RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_VAL,data) ) goto error;

	return 0;
error:
	return 1;
}


//3//////// Set GPIO3 "OUT"  => Turn ON/OFF Tuner Power
//3//////// Set GPIO3 "IN"      => Button  Wake UP (USB IF) , NO implement in rtl2832u linux driver

static int gpio3_out_setting()
{
	int			data;

	// GPIO3_PAD Pull-HIGH, BIT76
	data = 2;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_CFG0_BIT67,data) ) goto error;

	// GPO_GPIO3 = 1, GPIO3 output value = 1 
	if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_VAL, &data) ) goto error;		
	data |= BIT3;		//set bit3 to 1
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_VAL,data) ) goto error;

	// GPD_GPIO3=0, GPIO3 output direction
	data = 0;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT3,data) ) goto error;

	// GPOE_GPIO3=1, GPIO3 output enable
	data = 1;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT3,data) ) goto error;

	//BTN_WAKEUP_DIS = 1
	data = 1;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_USB_CTRL_BIT5,data) ) goto error;

	return 0;
error:
	return 1;
}

int usb_epa_fifo_reset()
{
	int	data;
	//3 reset epa fifo:
	//3[9] Reset EPA FIFO
	//3 [5] FIFO Flush,Write 1 to flush the oldest TS packet (a 188 bytes block)

	data = 0x0210;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_USB_EPA_CTL,data) ) goto error;

	data = 0xffff;
	if ( read_usb_sys_register( RTD2831_RMAP_INDEX_USB_EPA_CTL,&data) ) goto error;

	if( (data & 0xffff) != 0x0210)
	{
		printf("Write error RTD2831_RMAP_INDEX_USB_EPA_CTL = 0x%x\n",data);
	 	goto error;	
	}

	data=0x0000;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_USB_EPA_CTL,data) ) goto error;

	data = 0xffff;
	if ( read_usb_sys_register( RTD2831_RMAP_INDEX_USB_EPA_CTL,&data) ) goto error;

	if( ( data  & 0xffff) != 0x0000)
	{
		printf("Write error RTD2831_RMAP_INDEX_USB_EPA_CTL = 0x%x\n",data);
	 	goto error;	
	}

	return 0;

error:
	return 1;

}



static int usb_init_bulk_setting()
{
	int					data;		
	//3 1.FULL packer mode(for bulk)
	//3 2.DMA enable.
	if ( read_usb_sys_register( RTD2831_RMAP_INDEX_USB_SYSCTL, &data) ) goto error;

	data &=0xffffff00;
	data |= 0x09;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_USB_SYSCTL, data) ) goto error;

	data=0;
	if ( read_usb_sys_register( RTD2831_RMAP_INDEX_USB_SYSCTL, &data) ) goto error;
      
	if((data&0xff)!=0x09)  
	{
		printf("Open bulk FULL packet mode error!!\n");
	 	goto error;
	}

	//3check epa config,
	//3[9-8]:00, 1 transaction per microframe
	//3[7]:1, epa enable
	//3[6-5]:10, bulk mode
	//3[4]:1, device to host
	//3[3:0]:0001, endpoint number
	data = 0;
	if ( read_usb_sys_register( RTD2831_RMAP_INDEX_USB_EPA_CFG, &data) ) goto error;                
	if((data&0x0300)!=0x0000 || (data&0xff)!=0xd1)
	{
		printf("Open bulk EPA config error! data=0x%x \n" , data);
	 	goto error;	
	}

	//3 EPA maxsize packet 
	//3 512:highspeedbulk, 64:fullspeedbulk. 
	//3 940:highspeediso,  940:fullspeediso.

	//3 get info :HIGH_SPEED or FULL_SPEED
	if ( read_usb_sys_register( RTD2831_RMAP_INDEX_USB_STAT, &data) ) goto error;	
	if(data&0x01)  
	{
		data = 0x00000200;
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_USB_EPA_MAXPKT, data) ) goto error;

		data=0;
		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_USB_EPA_MAXPKT, &data) ) goto error;
	                      
		if((data&0xffff)!=0x0200)
		{
			printf("Open bulk EPA max packet size error!\n");
		 	goto error;
		}

		printf("HIGH SPEED\n");
	}
	else 
    	{
		data = 0x00000040;
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_USB_EPA_MAXPKT, data) ) goto error;

		data=0;
		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_USB_EPA_MAXPKT, &data) ) goto error;
	                      
		if((data&0xffff)!=0x0200)
		{
			printf("Open bulk EPA max packet size error!\n");
		 	goto error;
		}
		
		printf("FULL SPEED\n");
	}	
	
	return 0;

error:	
	return 1;
}


static int usb_init_setting()
{
	int					data;
	if ( usb_init_bulk_setting() ) goto error;
	//3 change fifo length of EPA 
	data = 0x00000014;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_USB_EPA_FIFO_CFG, data) ) goto error;
	data = 0xcccccccc;
	if ( read_usb_sys_register( RTD2831_RMAP_INDEX_USB_EPA_FIFO_CFG, &data) ) goto error;
	if( (data & 0xff) != 0x14)
	{
		printf("Write error RTD2831_RMAP_INDEX_USB_EPA_FIFO_CFG =0x%x\n",data);
	 	goto error;
	}

	if ( usb_epa_fifo_reset() ) goto error;
	return 0;

error: 
	return 1;	
}



static int suspend_latch_setting(unsigned char	resume)
{
	int					data;
	if (resume)
	{
		//3 Suspend_latch_en = 0  => Set BIT4 = 0 
		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1, &data) ) goto error;		
		data &= (~BIT4);	//set bit4 to 0
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1,data) ) goto error;
	}
	else
	{
		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1, &data) ) goto error;		
		data |= BIT4;		//set bit4 to 1
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1,data) ) goto error;
	}

	return 0;
error:
	return 1;

}





//3////// DEMOD_CTL1  => IR Setting , IR wakeup from suspend mode
//3////// if resume =1, resume
//3////// if resume = 0, suspend


static int 
demod_ctl1_setting(unsigned char resume)
{
	int					data;	
	if(resume)
	{
		// IR_suspend	
		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1, &data) ) goto error;		
		data &= (~BIT2);		//set bit2 to 0
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1,data) ) goto error;

		//Clk_400k
		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1, &data) ) goto error;		
		data &= (~BIT3);		//set bit3 to 0
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1,data) ) goto error;
	}
	else
	{
		//Clk_400k
		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1, &data) ) goto error;		
		data |= BIT3;		//set bit3 to 1
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1,data) ) goto error;

		// IR_suspend		
		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1, &data) ) goto error;		
		data |= BIT2;		//set bit2 to 1
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL1,data) ) goto error;
	}
	
	return 0;
error:
	return 1;

}


static int demod_ctl_setting(unsigned char	resume,	unsigned char  on)
{
	int	data;
	unsigned char	tmp;
			
	if(resume)
	{
		// PLL setting
		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL, &data) ) goto error;		
		data |= BIT7;		//set bit7 to 1
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL,data) ) goto error;
		

		//2 + Begin LOCK
		// Demod  H/W Reset
		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL, &data) ) goto error;		
		data &= (~BIT5);	//set bit5 to 0
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL,data) ) goto error;

		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL, &data) ) goto error;		
		data |= BIT5;		//set bit5 to 1
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL,data) ) goto error;
		
		//3 reset page chache to 0 		
		if ( read_demod_register(RTL2832_DEMOD_ADDR, 0, 1, &tmp, 1 ) ) goto error;	
		//2 -End LOCK

		// delay 5ms
		platform_wait(5);


		if(on)
		{
			// ADC_Q setting on
			if ( read_usb_sys_register(RTD2831_RMAP_INDEX_SYS_DEMOD_CTL, &data) ) goto error;		
			data |= BIT3;		//set bit3 to 1
			if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL,data) ) goto error;

			// ADC_I setting on
			if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL, &data) ) goto error;		
			data |= BIT6;		//set bit3 to 1
			if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL,data) ) goto error;
		}
		else
		{
			// ADC_I setting off
			if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL, &data) ) goto error;		
			data &= (~BIT6);		//set bit3 to 0
			if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL,data) ) goto error;

			// ADC_Q setting off
			if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL, &data) ) goto error;		
			data &= (~BIT3);		//set bit3 to 0
			if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL,data) ) goto error;		
		}
	}
	else
	{

		// ADC_I setting
		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL, &data) ) goto error;		
		data &= (~BIT6);		//set bit3 to 0
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL,data) ) goto error;

		// ADC_Q setting
		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL, &data) ) goto error;		
		data &= (~BIT3);		//set bit3 to 0
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL,data) ) goto error;

		// PLL setting
		if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL, &data) ) goto error;		
		data &= (~BIT7);		//set bit7 to 0
		if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL,data) ) goto error;

	}

	return 0;
error: 
	return 1;	

}


static int gpio1_output_enable_direction()
{
	int data;
	// GPD_GPIO1=0, GPIO1 output direction
	data = 0;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT1,data) ) goto error;

	// GPOE_GPIO1=1, GPIO1 output enable
	data = 1;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT1,data) ) goto error;

	return 0;
error:
	return 1;
}


static int gpio6_output_enable_direction()
{
	int data;
	// GPD_GPIO6=0, GPIO6 output direction
	data = 0;
	if ( write_usb_sys_register(RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT6,data) ) goto error;

	// GPOE_GPIO6=1, GPIO6 output enable
	data = 1;
	if ( write_usb_sys_register(RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT6,data) ) goto error;

	return 0;
error:
	return 1;
}

static void set_gpio4(int b_gpio4)
{
	int			data;

	 read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_VAL, &data); 		
	if(b_gpio4==0)		data &= ~(BIT4);   //set bit4 to 0
	else			data |= BIT4;		//set bit4 to 1	
	
   write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_VAL,data) ;
}


static int gpio5_output_enable_direction()
{
	int data;
	// GPD_GPIO5=0, GPIO5 output direction
	data = 0;
	if ( write_usb_sys_register(RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT5,data) ) goto error;

	// GPOE_GPIO5=1, GPIO5 output enable
	data = 1;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT5,data) ) goto error;

	return 0;
error:
	return 1;
}


int  rtl2832_hw_reset()
{
	int					data;
	unsigned char			tmp;
	
    // Demod  H/W Reset
	if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL, &data) ) goto error;		
			data &= (~BIT5);	//set bit5 to 0
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL,data) ) goto error;

	if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL, &data) ) goto error;		
			  data |= BIT5;		//set bit5 to 1
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_DEMOD_CTL,data) ) goto error;
	
	//3 reset page chache to 0 		
	if ( read_demod_register( RTL2832_DEMOD_ADDR, 0, 1, &tmp, 1 ) ) goto error;	

	// delay 5ms
	platform_wait(5);
	return 0;

error:
	return -1;
}


#define	USB_EPA_CTL	0x0148

 int rtl2832u_streaming_ctrl( int onoff)
{
	unsigned char data[4];	
	//3 to avoid  scanning  channels loss
	if(onoff)
	{
		data[0] = data[1] = 0;		
		if ( write_usb_sys_char_bytes(  RTD2832U_USB , USB_EPA_CTL , data , 2) ) goto error;				
	}
	else
	{
		data[0] = 0x10;	//3stall epa, set bit 4 to 1
		data[1] = 0x02;	//3reset epa, set bit 9 to 1
		if ( write_usb_sys_char_bytes(  RTD2832U_USB , USB_EPA_CTL , data , 2) ) goto error;		
	}

	return 0;
error: 
	return -1;
}



int rtl2832_GPIO4_hw_reset()
{

	int data;

	// GPIO4_PAD Pull-HIGH, BIT10
	data = 0;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_CFG1_BIT01,data) ) goto error;
	
	// GPO_GPIO4 = 1, GPIO4 output value = 1 
	if ( read_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_VAL, &data) ) goto error;
	data |= BIT4;		//set bit4 to 1
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_VAL,data) ) goto error;
	// GPD_GPIO4=0, GPIO4 output direction
	data = 0;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_DIR_BIT4,data) ) goto error;
	// GPOE_GPIO4=1, GPIO4 output enable
	data = 1;
	if ( write_usb_sys_register( RTD2831_RMAP_INDEX_SYS_GPIO_OUTPUT_EN_BIT4,data) ) goto error;
	set_gpio4(1);
	platform_wait(100);
	set_gpio4(0);
	platform_wait(300);
	set_gpio4(1);
	platform_wait(200);

	return 0;
error:
	return 1;
}

static unsigned int pid[1] = {0x1fff};
int rtl2832_init()
{
	rtl2832_InitRegTable();
	if( usb_init_setting() ) goto error;
	if(rtl2832_GPIO4_hw_reset()) goto error;
	if( gpio3_out_setting() ) goto error;				//3Set GPIO3 OUT	
	if( demod_ctl1_setting(1) ) goto error;		//3	DEMOD_CTL1, resume = 1
	if( set_demod_power(1) ) goto error;			//3	turn ON demod power
	if( suspend_latch_setting(1) ) goto error;		//3 suspend_latch_en = 0, resume = 1 					
	if( demod_ctl_setting(1, 0) ) goto error;		//3 DEMOD_CTL, resume =1; ADC on
	if( SetTsExt()) goto error;
//	if(rtl2832_pid_filter_ctr(0)) goto error;
//	if(rtl2832_pid_filter(pid,1)) goto error;
	printf("rtl2832_init Succeed.\n");
	return 0;
error:
	return -1;
}


int rtl2832_i2c_Rep(int On_Off)
{
	unsigned char i2c_repeater=0;
	if(On_Off==YES)
	{
		if(read_demod_register( RTL2832_DEMOD_ADDR, 1, 1, &i2c_repeater, 1 )) goto error;
		i2c_repeater |= BIT3;	
		if(write_demod_register( RTL2832_DEMOD_ADDR, 1, 1, &i2c_repeater, 1 )) goto error;
	}
	else
	{
		if(read_demod_register( RTL2832_DEMOD_ADDR, 1, 1, &i2c_repeater, 1 )) goto error;
		i2c_repeater &= (~BIT3);	

		if(write_demod_register( RTL2832_DEMOD_ADDR, 1, 1, &i2c_repeater, 1 )) goto error;
	}
	return 0;

error:
	return 1;
}

void ResetI2C(void)
{
		unsigned char i2c_repeater=0x10;
		read_demod_register( RTL2832_DEMOD_ADDR, 1, 1, &i2c_repeater, 1 );
		i2c_repeater &= (~BIT3);	
		write_demod_register( RTL2832_DEMOD_ADDR, 1, 1, &i2c_repeater, 1 );
}


static int rtl2832_read_ber(	u32*	ber)
{
	unsigned  long ber_num, ber_dem;
	/*
	if(GetBer( &ber_num , &ber_dem) ) 
	{
		*ber = 19616;
		goto error;
	}
	*ber =  ber_num;
	*/	
	return 0;	
error:	
	return -1;
}


int rtl2832_read_signal_strength(u16*	strength)
{	
	return 0;	
error:
	return -1;
}


int rtl2832_read_signal_quality(u32* quality)
{
	/*
	int _quality;
	if ( GetSignalQuality(  &_quality) )
	{
		*quality  = 0;		
		goto error;
	}

	*quality = _quality;
	*/
	return 0;	
error:
	return -1;	
}



static int rtl2832_read_snr(u16*	snr)
{

	long snr_num = 0;
	long snr_dem = 0;
	long _snr= 0;

/*
	if (GetSnrDb( &snr_num, &snr_dem) ) 
	{
		*snr = 0;
		goto error;
	}

	_snr = snr_num / snr_dem;

	if( _snr < 0 ) _snr = 0;

	*snr = _snr;
*/	
	return 0;	
error:	
	return -1;
}




/*
@parameter:mode  pid filter mode;
*/
int rtl2832_pid_filter_ctr(PID_FILTER_MODE mPassmode)
{
	unsigned char data;
	switch(mPassmode)
	{
		case PID_FILTER_PASS:
			data = 0xa8;
			if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x21, &data, LEN_1_BYTE)) 
				goto error;
			break;
		case PID_FILTER_ENABLE:
			data=0xe8;
			if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x21, &data, LEN_1_BYTE)) 
				goto error;
			break;
		default:
			data=0x08;
			if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x21, &data, LEN_1_BYTE)) 
				goto error;
			break;						
	}
	data = 0xff;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x22, &data, LEN_1_BYTE)) 
		goto error;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x23, &data, LEN_1_BYTE)) 
		goto error;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x24, &data, LEN_1_BYTE)) 
		goto error;                            
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x25, &data, LEN_1_BYTE)) 
		goto error;		
	printf("######################\n");		
	printf("set external pid filter control success!\n");
	printf("######################\n");
	return 0;
error:
	printf("######################\n");		
	printf("set external pid filter control fail!\n");
	printf("######################\n");	
	return -1;
}

/*
@parameter:pid  disable or enable pid;
@parameter:number pid count;

*/
int rtl2832_pid_filter(unsigned int* pid,int number)
{
	unsigned char data;
	unsigned char reg = 0x26;
	int i = 0;	
	for(i = 0;i < number;i++)
	{
		data = (pid[i] >> 8) & 0xff;
		if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, reg, &data, LEN_1_BYTE)) 
			goto error;
		data = (pid[i] >> 0) & 0xff;
		if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, reg + 1, &data, LEN_1_BYTE)) 
			goto error;
		reg += 2;		
	}
	printf("######################\n");		
	printf("set external pid filter success!\n");
	printf("######################\n");
	return 0;
error:
	printf("######################\n");	
	printf("set external pid filter fail!\n");
	printf("######################\n");
	return -1;
}

int SetTsExt(void)
 {
	unsigned char buf[2],data;
	//  deb_info("%s RTL2832P for RTL2836 and RTL2840 \n", __FUNCTION__);
	//3 1. Set IF_AGC Manual and Set IF_AGC MAX VAL
	buf[0]=0x5F;
	buf[1]=0xFF;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_1,  0x0c,  buf, LEN_2_BYTE)) 
		goto error;
	//3 2. PIP Setting
#if 1
	data = 0xa8;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x21, &data, LEN_1_BYTE)) 
		goto error;
#endif
	data = 0x60;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x61, &data, LEN_1_BYTE)) 
		goto error;
	data = 0x18;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0xbc, &data, LEN_1_BYTE)) 
		goto error;
		
	data = 0x00;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x62, &data, LEN_1_BYTE)) 
		goto error;
	data = 0x00;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x63, &data, LEN_1_BYTE)) 
		goto error;
	data = 0x00;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x64, &data, LEN_1_BYTE)) 
		goto error;                            
	data = 0x00;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x65, &data, LEN_1_BYTE)) 
		goto error;
#if 1	
	//3 +PIP filter Reject = 0x1FFF
	data = 0x01;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x22, &data, LEN_1_BYTE)) 

		goto error;
	data = 0x1f;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x26, &data, LEN_1_BYTE)) 
		goto error;
	data = 0xff;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x27, &data, LEN_1_BYTE)) 
		goto error;
#endif
	//3 -PIP filter Reject = 0x1FFF
	data = 0x7f;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_1, 0x92, &data, LEN_1_BYTE)) 
		goto error;
	data = 0xf7;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_1, 0x93, &data, LEN_1_BYTE)) 
		goto error;
	data = 0xff;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_1, 0x94, &data, LEN_1_BYTE)) 
		goto error;

	rtl2832_SoftwareReset();

	return 0;
error:
	return 1;
}

int disable_pid_filter(void)
{
#if 1
	unsigned char data = 0xe8;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x21, &data, LEN_1_BYTE)) 
		goto error;
#endif
#if 1	
	//3 +PIP filter Reject = 0x1FFF
	data = 0x01;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x22, &data, LEN_1_BYTE)) 
		goto error;
	data = 0x1f;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x26, &data, LEN_1_BYTE)) 
		goto error;
	data = 0xff;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x27, &data, LEN_1_BYTE)) 
		goto error;
	//clear the pid register
	data = 0x00;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x23, &data, LEN_1_BYTE)) 
		goto error;
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x24, &data, LEN_1_BYTE)) 
		goto error;                            
	if(write_demod_register( RTL2832_DEMOD_ADDR,  PAGE_0, 0x25, &data, LEN_1_BYTE)) 
		goto error;
#endif
	return 0;
error:
	printf("######################\n");	
	printf("set external pid filter fail!\n");
	printf("######################\n");
	return -1;

}

