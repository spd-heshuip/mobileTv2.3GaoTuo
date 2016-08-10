#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <linux/fd.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include "../GaoTuo/atbm888x.h"

#define SERV_PORT 6000

extern int UDP_FD;//UDP句柄
extern int Tcp_Sock;
extern int TCP_FD;
extern bool TCP_Flag;
extern bool PID_FILTER_FLAG;
extern bool parseOkFlag;

extern pthread_t tid;//UDP TS流线程id


extern int disable_pid_filter(void);
extern DTMB_QAM_INDEX  ATBM_GetQamIndex(void);
extern int Get_SSI_Info(unsigned int type);//获取信号强度
extern int Get_Quality_Info();//获取信号质量
extern double Get_BER_Info();
extern double Get_PER_After_BCH();
extern void StartTsThread(void);//开始传输UDP TS流

extern int setProgramAndPidFilter();

extern int sony_cxd2837_init(void);
extern int rtl2832_pid_filter_ctr(int mPassmode);//PID过滤模式
extern int rtl2832_pid_filter(unsigned int* pid,int number);//PID过滤

struct sockaddr_in UDP_Cli;
struct sockaddr_in cli_addr;
struct sockaddr_in New_Cli_Addr;
struct sockaddr_in TCP_Server;

bool Connect_Flag = false;//是否有客户端链接标志位
bool PTHREAD_FLAG = false;//UDP TS流传输标志位
pthread_t nAck_Thread;//登录回应线程
pthread_t HeartBeartThread;//心跳包线程
int AcceptFlag = 0;

static bool Lock_Freq_Flag = false;//锁定频点标志位
static bool Login_Flag = false;//登录标志位
static bool HeartBeartFlag = false;//心跳包线程标志位
static bool ISDBT_MODE_FLAG = true;//ISDBT模式标志位
static bool isCheckContinue = false;

unsigned int Pid_List[32]= {0};
int pid_count = 0;

static pthread_attr_t attr;
static pthread_attr_t attr2;
static pthread_attr_t attr3;
static struct sched_param param;

int New_Sockfd;
int sockfd,connect_fd;
int HB_Sock;
int RTP_Port = 0;
int Tv_type;
int freq;
int bandwidth;
int plp;
int isScan;
int programNumber = 0;

int lastLockFreq = 0;


typedef enum _REQUEST_TYPE
{
	LOGIN_REQ = 1,
	LOGOUT_REQ,
	TUNE_REQ,
	SIGNAL_STATUS_REQ,
	STOP_STREAM_REQ,
	PID_FILTER_REQ,
	SWITCH_MODE_REQ,
	START_TS_SEND_REQ,
	CHECK_LOCK_REQ,	
	RELOCK_REQ
}REQUEST_TYPE;

#endif
