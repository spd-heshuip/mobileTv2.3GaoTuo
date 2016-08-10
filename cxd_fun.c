#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "rtl2832u_io.h"
#include "libusb.h"
#include "lusb0_usb.h"

extern void Delay(int sec,int usec);
extern void ResetI2C(void); 
extern unsigned char ATBM88xx_GetPower(void);


uint32_t WaitLockTime(uint32_t ms)
{
	Delay(0,ms);
	return(0);
}

signed char I2cWrite8( unsigned char chipaddr,unsigned char Reg,unsigned char *Regda,unsigned int dalen,unsigned int unnecessary){ 

	unsigned char Reg_r = Reg;
	write_rtl2832_stdi2c(chipaddr,&Reg_r,1);
	write_rtl2832_stdi2c(chipaddr,Regda,dalen);
	return 0;
}
signed char I2cRead8(unsigned char chipaddr,unsigned char Reg,unsigned char *Regda,unsigned int dalen){
	unsigned char Reg_r=Reg;
	write_rtl2832_stdi2c(chipaddr,&Reg_r,1);
	read_rtl2832_stdi2c(chipaddr,Regda,dalen);
	return 0;
}
signed char I2cRead16_NoStop(unsigned char chipaddr,unsigned int Reg,unsigned char *Regda,unsigned int dalen){
	unsigned char Reg_r[2];
	Reg_r[0]=Reg>>8;
	Reg_r[1]=Reg;
	write_rtl2832_stdi2c(chipaddr,&Reg_r[0],2);
	read_rtl2832_stdi2c(chipaddr,Regda,dalen);
	return 0;
}


uint32_t CS_OS_time_now()
{
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv,&tz);
	return tv.tv_sec;
}

/*
** type: 0代表DVB-C，1代表DVB-T，2代表DVB-T2
*/
int Get_SSI_Info(unsigned int type)
{
	ResetI2C();
	int ret = ATBM88xx_GetPower();
	if(ret > 10)
		ret = -90; 
	return ret;	
}

int Get_Quality_Info()
{
	int ret = ATBM88xx_GetQuality();
	return ret;
}



