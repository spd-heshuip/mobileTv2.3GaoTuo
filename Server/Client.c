#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <unistd.h>
//#include <curses.h>
#include <fcntl.h>
#include <errno.h>


#define m_nServerPort 6000
#define m_szServerIP "192.168.1.1"
//#define m_szServerIP "192.168.10.225"
int m_nMyRecvPort = 8000;
int sockfd;

/*延时函数*/
void Delay(int sec,int usec)
{
	struct timeval time;
	time.tv_sec = sec;
	time.tv_usec = usec;
	select(0,NULL,NULL,NULL,&time);
}

bool GetParamValue( const char *xml, const char *paramName, const char *start, const char *end, char *value )
{
	const char *p = strstr( xml, paramName ) ;
	if( p == NULL )
		return false ;
	
	const char *pStart = strstr( p, start ) ;
	if( pStart == NULL )
		return false ;

	if( strlen(pStart) <= strlen(start) )
		return false ;

	const char *pEnd = strstr( pStart+strlen(start), end ) ;
	if( pEnd == NULL )
		return false ;

	int len = pEnd-pStart-strlen(start);
	if( len > 256 )
		len = 256 ;

	strncpy( value, pStart+strlen(start), len ) ;
	value[len] = '\0';

	return true ;
}

int GetResponseRetValue( const char*xml)
{
	char szValue[256]={0};
	bool bOK = GetParamValue( xml, "ret", "\"", "\"", szValue ) ;
	if( !bOK )
		return -1 ;

	return atoi(szValue) ;
}

int activate_nonbloack(int fd)
{
	int ret = 0;
	int flags = fcntl(fd,F_GETFL);
	if(flags == -1)
	{
		printf("fcntl error\n");
		return -1;
	}
	flags |= O_NONBLOCK;
	ret = fcntl(fd,F_SETFL,flags);
	if(ret == -1)
	{
		printf("fcntl error\n");
		return -1;
	}

}

int deactivate_noblock(int fd)
{
	int ret = 0;
	int flags = fcntl(fd,F_GETFL);
	if(flags == -1)
		return -1;
	flags &= ~O_NONBLOCK;
	ret = fcntl(fd,F_SETFL,flags);
	if(ret == -1)
		return -1;
}

int connect_timeout(int fd,struct sockaddr_in* addr,unsigned int wait_seconds)
{
	int ret = 0;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	
	if(wait_seconds > 0)
		activate_nonbloack(fd);
	
	ret = connect(fd,(struct sockaddr*)addr,addrlen);
	if(ret < 0 && errno == EINPROGRESS)
	{
		struct timeval timeout;	
		fd_set connectSet;		

		FD_ZERO(&connectSet);
		FD_SET(fd,&connectSet);	
		/*设置超时等待时间*/
		timeout.tv_sec = wait_seconds;
		timeout.tv_usec = 0;
		do
		{
			ret = select(fd+1,NULL,&connectSet,NULL,&timeout);
		}while(ret < 0 && errno == EINTR);
		
		if(ret == 0)
		{
			errno = ETIMEDOUT;
			return -1;
		}else if(ret < 0)
			return -1;
		else if(ret == 1)
		{	
			int err = 0;
			socklen_t socklen = sizeof(err);
			int sockoptret = getsockopt(fd,SOL_SOCKET,SO_ERROR,&err,&socklen);
			if(sockoptret == -1)
				return -1;
			if(err == 0)
				ret = 0;
			else
			{
				errno = err;
				ret = -1;
			}
		}
	}

	if(wait_seconds > 0)
		deactivate_noblock(fd);
	return ret;
}

int ConnectToServer()
{
	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sockfd < 0 )
		return -1 ;

	struct sockaddr_in serv_addr;
	memset( &serv_addr, 0, sizeof(serv_addr) ) ;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(m_nServerPort);
	serv_addr.sin_addr.s_addr = inet_addr(m_szServerIP);

	int nRecvBuf = 1024*8;

	setsockopt( sockfd, SOL_SOCKET, SO_RCVBUF,(const char*)&nRecvBuf, sizeof(int) );

	struct timeval nTimeOut = {6,0};
	int ret = setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&nTimeOut, sizeof(nTimeOut) );
	if(ret < 0)
		return -1;
	ret = setsockopt( sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&nTimeOut, sizeof(nTimeOut) );
	if(ret < 0)
		return -1;

	ret = connect_timeout( sockfd, &serv_addr,10) ;
	if( ret < 0 )
	{
		close(sockfd);
		return -2 ;
	}

	return sockfd ;
}

bool ExchangeMessage( const char *pSendMessage, char *pRecvMessage ) 
{
	if( sockfd < 0 )
		return false;

	printf( "Send:%s\n", pSendMessage ) ;
	int ret = send( sockfd, pSendMessage, strlen(pSendMessage), 0 ) ;
	if( ret < 0 )
		return false ;
//	Delay(10,0);
	ret = recv( sockfd, pRecvMessage, 1024, 0 ) ;
	if( ret <= 0 )
	{
		close( sockfd ) ;
		sockfd = -1 ;
		return false ;
	}

	pRecvMessage[ret] = '\0' ;

	printf( "Receive:%s\n", pRecvMessage ) ;

	if(strncmp( pRecvMessage, "<msg", 4 ) != 0 )
	{
		return false ;
	}
	if(strncmp( &pRecvMessage[ret-6], "</msg>", 6 ) != 0 )
	{
		return false ;
	}

	return true ;
}

bool Login()
{
	char szReq[1024] = {0};
	char szResp[1024] = {0};

	sprintf( szReq, "<msg type=\"login_req\"><params protocol=\"UDP\" port=\"%d\"/></msg>", m_nMyRecvPort ) ;

	bool bOK = ExchangeMessage( szReq, szResp ) ;
	if( !bOK )
		return false ;

	int ret = GetResponseRetValue( szResp ) ;
	if( ret != 0 )
	{
		return false ;
	}
	char szDevInfo[256];
	bOK = GetParamValue( szResp, "deviceinfo", " ", "/>", szDevInfo ) ;
	printf("devices info:%s\n",szDevInfo);
	return true ;
}

bool Logout()
{
	char szReq[1024]={0};
	char szResp[1024];

	sprintf( szReq, "<msg type=\"logout_req\"></msg>") ;
	bool bOK = ExchangeMessage( szReq, szResp ) ;
	if( !bOK )
		return false ;

	int ret = GetResponseRetValue( szResp ) ;
	if( ret != 0 )
		return false ;

	return true ;
}

bool LockFrequencyPoint( char *szTVtype, int freq, int bandwidth, int plp,int programNumber)
{
	char szReq[1024]={0};
	char szResp[1024];

	sprintf( szReq, "<msg type=\"tune_req\"><params tv_type=\"%s\" freq=\"%d\" bandwidth=\"%d\" plp=\"%d\" isScan=\"%d\" programNumber=\"%d\"/></msg>",szTVtype,freq,bandwidth,plp,1,programNumber);
	bool bOK = ExchangeMessage( szReq, szResp ) ;
	if( !bOK )
		return false ;

	int ret = GetResponseRetValue( szResp ) ;
	if( ret != 0 )
		return false ;

	return true ;
}

bool StopTsSend()
{
	char szReq[1024] = {0};
	char szResp[1024];

	sprintf(szReq,"<msg type=\"stop_stream_req\"></msg>");
	bool bOK = ExchangeMessage(szReq,szResp);
	if(!bOK)
		return false;
	
	int ret = GetResponseRetValue(szResp);
	if(ret != 0)
		return false;
	return true;
}

bool GetSignalStatus()
{
	char szReq[1024]={0};
	char szResp[1024];

	sprintf(szReq, "<msg type=\"signal_status_req\"></msg>") ;
	bool bOK = ExchangeMessage( szReq, szResp ) ;
	if( !bOK )
		return false ;

	char Signal_Status[500];
	bOK = GetParamValue(szResp,"signal_status"," ","/>",Signal_Status);
	if(!bOK )
		return false ;
	printf("Signal_Status:%s\n",Signal_Status);

	return true ;
}

bool GetScanChanelStatus()
{
	char szReq[1024] = {0};
	char szResp[1024];

	sprintf(szReq,"<msg type=\"Scan_Channel_req\"></msg>");
	bool bOK = ExchangeMessage( szReq,szResp );
	if(!bOK)
		return false;

	int ret = GetResponseRetValue(szResp);
	if(ret == -1)
	{
		printf("all channel is no signal!\n");
		return false;
	}

	char Freq[10];
	bOK = GetParamValue(szResp,"freq","\"","\"",Freq);
	if( !bOK )
		return false;
	
	int freq = atoi(Freq);
	printf("Signal Channel Freq:%d\n",freq);

	bOK = LockFrequencyPoint("DVB-T2",freq * 1000,8000,0,0);
	if(bOK)
		printf("Lock FrequencyPoint success!\n");
	else
		printf("Lock FrequencyPoint fail!\n");

	return true;
}

bool SendPidFilter(unsigned char* pid_list)
{
	char szReq[1024] = {0};
	char szResp[1024];

	sprintf(szReq,"<msg type=\"Pid_Filter_req\"><params pid_list=\"%s\"></msg>",pid_list);
	bool bOK = ExchangeMessage(szReq,szResp);
	if(!bOK)
		return false;

	int ret = GetResponseRetValue(szResp);
	if(ret == -1)
	{
		printf("set pid filter fail!\n");
		return false;
	}

	return true;
}
#define DVB_T 1
#define ISDBT 2
/**/
bool SwitchMode(int mode)
{
	char szReq[1024] = {0};
	char szResp[1024];
	if(mode == 1)	
		sprintf(szReq,"<msg type=\"Switch_Mode_req\"><params mode=\"DVB-T2\"></msg>");
	else
		sprintf(szReq,"<msg type=\"Switch_Mode_req\"><params mode=\"ISDBT\"></msg>");	

	bool bOK = ExchangeMessage( szReq,szResp );
	if(!bOK)
		return false;

	int ret = GetResponseRetValue(szResp);
	if(ret < 0)
	{
		printf("switch mode fail!\n");
		return false;
	}

	return true;
}

void *Ack(void * arg)
{
	int sock = *((int *)arg);	
	int ret = 0;
	unsigned char buf[256];
	struct timeval time;
	int maxfd = sock;
	unsigned char* send_buf = "HeartBeartAck";
	
	fd_set fds;

	while(1)
	{
		
		FD_ZERO(&fds);
		FD_SET(sock,&fds);

		time.tv_sec = 15;
		time.tv_usec = 0;

		ret = select(maxfd+1,&fds,NULL,NULL,&time);
		if(ret < 0)
		{
			printf("select option fail!\n");
			return NULL;
		}
		else if(ret == 0)
		{
			printf("time out! server down! please check the mobile tv box\n");
			FD_CLR(sock,&fds);
			close(sock);
			return NULL;
		}
		if(FD_ISSET(sock,&fds))
		{
			memset(buf,0,256);			
			ret = recv(sock,buf,256,0);
			if(ret <= 0)
			{
				printf("Server Close!\n");
				close(sock);				
				return NULL;
			}
			printf("###################\n");
			printf("recv heartbeat msg:%s\n",buf);
			printf("###################\n");
			ret = send(sock,send_buf,strlen(send_buf),0);
			if(ret < 0)
			{
				printf("send heartbeart ack fail!\n");
				return NULL;
			}
			printf("#######################\n");
			printf("send heartbeat ack success!\n");
			printf("#######################\n");
		}
	}
}

int ConnectToServer2()
{
	int ret = 0;
	int sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sockfd < 0 )
		return -1 ;

	int nRecvBuf = 1024*1024*2;
	setsockopt( sockfd, SOL_SOCKET, SO_RCVBUF,(const char*)&nRecvBuf, sizeof(int) );

	int nTimeOut = 1000*5;
	setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&nTimeOut, sizeof(nTimeOut) );
	setsockopt( sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&nTimeOut, sizeof(nTimeOut) );

	int Reuse = 1;
	ret = setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&Reuse,sizeof(int));
	if(ret < 0)	
		return -1;	

	struct sockaddr_in serv_addr;
	memset( &serv_addr, 0, sizeof(serv_addr) ) ;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8000);
	serv_addr.sin_addr.s_addr = inet_addr(m_szServerIP);

	ret = connect( sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr) ) ;
	if( ret < 0 )
	{
		close(sockfd);
		return -1 ;
	}

	return sockfd ;
}

static int Thread_Flag = 1;
FILE* fd;
void *RecvTs(void* arg)
{
	int sock = *((int *)arg);
	unsigned char buf[1024] = {0};
	int length = 0;
#if 1
	fd = fopen("ts.ts", "wb+");
	if (fd == NULL)
		return NULL;
#endif	
	while(Thread_Flag)
	{
		length = recv(sock,buf,1024,0);
		if(length < 0)
		{
			printf("recv data fail!\n");
			close(sock);
			return;
		}
		fwrite(buf, 1, length, fd);
	}
	close(sock);
}

int main()
{
#if 1
	int sock = ConnectToServer();
	sockfd = sock;
	bool state = Login();
	if(state)
		printf("Login success!\n");
	else
	{
		printf("Login fail!\n");
		close(sockfd);
		return -1;
	}
	int buf;;
	int len = 4;
	getsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO,(char *)&buf,&len);
	printf("timeout option = %d\n",buf);
#if 1
	sock = ConnectToServer();
	pthread_t AckHeartBeat;
	int ret = pthread_create(&AckHeartBeat,NULL,Ack,(void *)&sock);
	if(ret < 0)
	{
		perror("create ack thread fail!\n");
		close(sock);
		close(sockfd);
		return -1;	
	}
	printf("HeartBeartThread create Success!\n");
#endif
#if 1
	int i = 0;
	int freq;
	printf("Please input the freq you need to lock:");
	scanf("%d",&freq);
	while(i < 1)
	{	
		state = LockFrequencyPoint("DTMB",freq,8000,0,1);
		if(state)
			printf("Lock FrequencyPoint success!\n");
		else
		{			
			printf("Lock FrequencyPoint fail!\n");
			close(sock);
			close(sockfd);
			return -1;		
		}		
		i++;
	}
#if 1
	pthread_t tcpRecvThread;
	int sock2 = ConnectToServer2();
	pthread_create(&tcpRecvThread,NULL,RecvTs,(void *)&sock2);
#endif
#endif
	sleep(5);		
	unsigned char* pid_list_1 = "10"; 
#if 0
	state = SendPidFilter(pid_list_1);
	if(state)
		printf("set pid filter success!\n");
	else
		printf("set pid filter fail!\n");
#endif
//	sleep(8);
#if 0
	state = StopTsSend();
	if(!state)	
		printf("************\n");
#endif

	while(1);
#if 0
	state = GetSignalStatus();
	if(state)
		printf("GetSignalStatus success!\n");
	else
		printf("GetSignalStatus fail!\n");
#endif
//	StopTsSend();

//	sleep(5);
		
//	state = LockFrequencyPoint("DVB-T2",freq,8000,0);
//	if(state)
//		printf("Lock FrequencyPoint success!\n");

#endif
	
#if 1
	state = Logout();
	if(state)
	{
		printf("Login out success!\n");
		close(sockfd);
		return 0;
	}
	else
		printf("Login  out fail!\n");
#endif
}
