#ifndef _USB_FUN_H_
#define _USB_FUN_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "libusb.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include "ParseSDT.h"
#include "ParsePAT.h"
#include "ParsePMT.h"
#include "Program.h"

#include "rtl2832u_fe.h"


static libusb_device_handle *pdev_handle;

#define CTRL_IN			(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN)
#define CTRL_OUT		(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT)

#define ERR_0 0
#define ERR_1 1
#define ERR_2 2

#define FW_BULKOUT_SIZE		61
#define EP_BULK_OUT			0X01
#define EP_BULK_IN			0X81
#define EP_BULK_TSIN		0X88
#define BUF_SIZE 64

#define SET_FILTER_SUCCESS 0
#define SET_FILTER_FAIL	-1
#define NO_TS_STREAM -2

int StopTsThread(void);

extern int UDP_FD;
extern int TCP_FD;
extern int Tcp_Sock;
extern int isScan;
extern int programNumber;

extern unsigned int Pid_List[32];
extern int pid_count;

extern struct sockaddr_in UDP_Cli;

extern libusb_device_handle* dev;

static unsigned char imgbuf[10][1024*32];
static int imgSize = 1024*32;
static unsigned char buf[1024*32];
static struct libusb_transfer *img_transfer[10] = {NULL};
static int cImg = 0;
static int bTsThread = 0;

extern int AcceptFlag;
extern int connect_fd;
extern bool PTHREAD_FLAG;
extern bool Connect_Flag;
extern pthread_t nAck_Thread;
extern int HB_Sock;
pthread_t tid;
bool TCP_Flag = false;

static pthread_attr_t attr3;
static struct sched_param param;
static int errCount = 0; 

static int ErrorCount = 0;
static int writeCount = 0 ;

FILE *fd;

bool PID_FILTER_FLAG = false;
bool parseOkFlag = false;

//PROGRAM_INFO astProgramInfo[PROGRAM_MAX] = {0};

extern int Tuner_Init(void);
extern int Create_Sock_Tcp(void);
extern void Delay(int sec,int usec);
extern int set_pthread_policy(pthread_attr_t* attr,int policy,struct sched_param* param);
extern int sendFailMessage(unsigned char* buf,int sock);

extern int disable_pid_filter(void);
extern int rtl2832_pid_filter(unsigned int* pid,int number);
extern int rtl2832_pid_filter_ctr(PID_FILTER_MODE mPassmode);

extern int IntToString(unsigned int* table,unsigned char* msg,int length);

extern void Get_Pid_List(char *xml);

extern int ParseTransportStream(FILE *pfTsFile,PROGRAM_INFO *astProgramInfo);

extern PROGRAM_INFO GetProgram(PROGRAM_INFO *pstProgramInfo, int iProgramCount, unsigned int uiServiceId);

#endif
