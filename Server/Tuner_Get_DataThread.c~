#include "Tuner_Get_DataThread.h"

int rtl2832_usb_init()
{
	int err = 0;
	if(err = usbi2cinit())
	{
		printf("usb i2c init fail!\n");
		return err;
	}	
	else
	{	
		err=rtl2832_init();
	}	
	return err;
}
/*Tuner初始化*/
int Tuner_Init()
{
	int ret = 0;
	unsigned char status;
	if(!rtl2832_usb_init())
	{
		printf("usb init success!\n");
#if 1
#if 0
		if(sony_cxd2837_CheckID())		
		{
			printf("Demod type is cxd2837!\n");
			DemMode = 37;
			ret = sony_cxd2837_init();
			if(ret != 0 )
				return -1;
		}
#endif
		if(/*Check_ATBM88xx_ID()*/1)
		{
			printf("Demod type is atbm8880!\n");			
			DemMode = 35;			
			ret = ATBM88xx_Init();
			if(ret != 0)
				return -1;
		}
		else
		{
			printf("######################################################\n");
			printf("did not check demod module! please check the hardware!\n");
			printf("######################################################\n");	
			return -1;	
		}
#endif				
	}
	else
		return -1;
	printf("Tuner Init success!\n");
}
/*/锁定频点*/
int Tuner_Lock(int Tv_type,int freq,int bandwidth,int plp)
{
	int ret = 0;
	if(DemMode == 37)
	{
		if(Tv_type == 2 || Tv_type == 1)
		{
			sony_cxd2837_DVBT2_tun(freq,bandwidth);		
			ret = sony_cxd2837_CheckLock(2);
			if(ret != 1)
			{
				sony_cxd2837_DVBT_tun(freq,bandwidth/1000);
				ret = sony_cxd2837_CheckLock(1);		
				if(ret != 1)
					return -1;						
			}
		}		
	}
	else if(DemMode == 35)
	{
		if(Tv_type == 3)
		{
			int i = 5;			
			ATBM88xx_ChannelLock(freq);
			Delay(0,5000);
			ret = ATBM88xx_CheckLock();
			if(ret != 1)
				return -1;
			ATBM_PPM_Test();	
		}	
	}
	
	
	return 0;
}

/*创建UDP传输*/
int Create_Sock_Udp()
{
	UDP_FD = socket(AF_INET,SOCK_DGRAM,0);	
	if(UDP_FD< 0)	
	{		
		printf("socket fail!\n");
		return -1;	
	}	

	memset(&UDP_Cli,0,sizeof(struct sockaddr_in));	
	UDP_Cli.sin_family = AF_INET;
	UDP_Cli.sin_port = htons(CLIENT_UDP_PORT);
#if 1
	UDP_Cli.sin_addr.s_addr = inet_addr(inet_ntoa(cli_addr.sin_addr));
#endif
//	UDP_Cli.sin_addr.s_addr = inet_addr(UDP_DEST);

#if 1
	int SndBuf = 1024 * 1024 * 2;
	int ret = setsockopt(UDP_FD,SOL_SOCKET,SO_SNDBUF,(const void*)&SndBuf,sizeof(int));
	if(ret < 0)
	{
		printf("setsockopt SNDBUF fail!\n");
		return -1;
	}
#endif
	printf("create sock udp success!\n");
	return 0;
}

static int TS_TCP_Option()
{
	int keeplive =1;

	int KeepIdle = 180;
	int KeepInterval = 180;  
	int KeepCount = 3;
	int SndBuf = 1024 * 1024 * 2;
	int ret = 0;

	struct timeval nTimeOut = {3,500};
	int Reuse = 1;
	int rcvBuf = 1460 * 60;

	ret = setsockopt(TCP_FD,SOL_SOCKET,SO_KEEPALIVE,(void *)&keeplive,sizeof(int));
	if(ret < 0)
		return -1;
	ret = setsockopt(TCP_FD,SOL_SOCKET,SO_SNDBUF,(const void*)&SndBuf,sizeof(int));
	if(ret < 0)	
		return -1;
#if 1
	ret = setsockopt(TCP_FD,SOL_SOCKET,SO_SNDTIMEO,(const void *)&nTimeOut,sizeof(nTimeOut));
	if(ret < 0)
		return -1;
#endif
	ret = setsockopt(TCP_FD,SOL_SOCKET,SO_REUSEADDR,(const void *)&Reuse,sizeof(Reuse));
	if(ret < 0)
		return -1;
	int len = 4;
	ret = getsockopt(TCP_FD,SOL_SOCKET,SO_SNDBUF,(void *)&SndBuf,&len);
	printf("set send buf value is:%d\n",SndBuf);
	return 0;
}

int accept_timeout(int fd,struct sockaddr_in * addr,int waitTimeOut)
{
	int ret = 0;
	if(waitTimeOut > 0)
	{
		struct timeval time;	
		fd_set acceptSet;		

		FD_ZERO(&acceptSet);
		FD_SET(fd,&acceptSet);	
		/*设置超时等待时间*/
		time.tv_sec = waitTimeOut;
		time.tv_usec = 0;
		do
		{	
			ret = select(fd+1,&acceptSet,NULL,NULL,&time);
		}
		while(ret < 0 && errno == EINTR);	
	
		if(ret == 0)
		{
			printf("Time Out!\n");
			errno = ETIMEDOUT;
			return -1;
		}
		else if(ret == -1)	
			return -1;
	}
	/*select*/

	socklen_t socklen = sizeof(struct sockaddr_in);
	if(addr != NULL)
		ret = accept(fd,(struct sockaddr *)addr,&socklen);
	else
		ret = accept(fd,NULL,NULL);
	
	return ret;
}


#if 1
/*创建TCP传输*/
int Create_Sock_Tcp()
{
	TCP_FD = socket(AF_INET,SOCK_STREAM,0);	
	if(TCP_FD< 0)	
	{		
		printf("socket fail!\n");
		return -1;	
	}	

	memset(&TCP_Server,0,sizeof(struct sockaddr_in));	
	TCP_Server.sin_family = AF_INET;
	TCP_Server.sin_port = htons(CLIENT_UDP_PORT);
	TCP_Server.sin_addr.s_addr = htonl(INADDR_ANY);

#if 1
	int ret = TS_TCP_Option();
	if(ret < 0)
	{
		printf("Send data's tcp socket set Option fail!\n");
		return -1;
	}
	
	if((bind(TCP_FD,(struct sockaddr *)&TCP_Server,sizeof(TCP_Server))) < 0)
	{
		perror("bind Send data's tcp socket fail!\n");
		exit(-1);
	}

	if((listen(TCP_FD,2)) < 0)
	{
		perror("Listen server socket fail!\n");
		close(TCP_FD);
		if(Tcp_Sock > 0)
		{
			close(Tcp_Sock);
			Tcp_Sock = -1;
		}
		TCP_FD = -1;
		return -1;
	}

	unsigned char pSendMessage[1024];
	sprintf(pSendMessage,"<msg type=\"tuner_ack\"><ack_info ret=\"0\" errmsg=\"OK\"/></msg>");
	ret = SendMessage(pSendMessage);
	if(ret < 0)
		return -1;

	int size_cliaddr = sizeof(cli_addr);
	struct sockaddr_in accept_Cli_Addr;
	
	printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
	printf("^^^^^^^waitting the client connect to server......^^^^^^^^^\n");
	printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
	Tcp_Sock = accept_timeout(TCP_FD,&TCP_Server,5);
	if(Tcp_Sock < 0)
	{	
		if(errno == EINTR)
			return -1;
		perror("accept fail!\n");
		close(TCP_FD);
		TCP_FD = -1;
		return -1;
	}
	AcceptFlag = 1;
	printf("#############################\n");
	printf("client accept tcp  ts thread!\n");
	printf("#############################\n");
	printf("create sock tcp success!\n");		

#endif	
	return 0;
}
#endif



