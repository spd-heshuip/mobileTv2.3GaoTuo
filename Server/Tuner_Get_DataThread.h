#ifndef _TUNER_GET_DATATHREAD_H_
#define _TUNER_GET_DATATHREAD_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/shm.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <linux/fb.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>



extern int  usbi2cinit();
extern  int rtl2832_init();

extern int sony_cxd2837_CheckID(void);
extern int sony_cxd2837_CheckLock(unsigned char sys);
extern int sony_cxd2837_DVBT2_tun(unsigned long int centerFreqKHz,unsigned int BW_kHz);
extern int sony_cxd2837_DVBT_tun(unsigned long int centerFreqKHz,unsigned int BW_MHz);
extern int sony_cxd2837_init(void);

extern unsigned char CXD2840_Check_Id(void);
extern int CXD2840_Init(void);
extern int CXD2840_Tune(unsigned char CXD2840_DTV_Mode,unsigned long int frequency);
extern int CXD2840_CheckTSLock(void);

extern unsigned char Check_ATBM88xx_ID(void);
extern int ATBM88xx_Init(void);

extern void ATBM88xx_ChannelLock(unsigned long FrequencyKHz);

extern unsigned char ATBM88xx_CheckLock(void);

extern unsigned char DemMode;

extern struct sockaddr_in cli_addr;
extern struct sockaddr_in UDP_Cli;

extern struct sockaddr_in TCP_Server;
#define CLIENT_UDP_PORT 8000

int UDP_FD;
int TCP_FD;
int Tcp_Sock;

int programNumber;

#define UDP_DEST "192.168.1.113"
/*USB芯片初始化*/
extern int connect_fd;
extern int Tv_type;
extern int AcceptFlag;
extern void Delay(int sec,int usec);



#endif
