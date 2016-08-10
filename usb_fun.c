#include "usb_fun.h"

int getProgramVideoAudioPids(PROGRAM_INFO *programInfos,int programNumber,unsigned char* pids,int count)
{
	PROGRAM_INFO astProgramInfo = {0};
	astProgramInfo = GetProgram(programInfos,count,programNumber);
	PROGRAM_INFO* p = &astProgramInfo;

	unsigned char audioId[1024] = {0};
	unsigned char pesPid[1024] = {0};
	unsigned char* nitSdtInfo = "0,17,18,20";

	int pidCount = IntToString(p->auiAudioPID,audioId,20);
	if(pidCount == 0)
		pidCount = -1;
	int pesPidLength = IntToString(p->pesPrivatePID,pesPid,50);
	if(pesPidLength == 0)
		pesPidLength = -1;

	sprintf(pids,"%d,%d,%d,%s,%s,%s",p->uiPMT_PID,p->uiVideoPID,p->pcrPID,audioId,pesPid,nitSdtInfo);
	
	return (pidCount) + (pesPidLength) + 7;
}

void programInfoToString(PROGRAM_INFO *astProgramInfo,unsigned char freqInfo[][1024],int length)
{
	unsigned char nitSdtInfo[4] = {0,17,18,20};	
	unsigned char info[1024] = {0};
	
	int i = 0;	
	for(i = 0;i<length;i++)
	{
		unsigned int audioId[1024] = {0};
		IntToString(astProgramInfo[i].auiAudioPID,audioId,20);
		printf("audiopid = %s\n",audioId);
		sprintf(info,"<msg type=\"programInfo\"><params programNumber=\"%d\" PmtPid=\"%d\" programName=\"%s\" VideoPid=\"%d\" AudioPid=\"%s\"/></msg>",astProgramInfo[i].uiProgramNumber,astProgramInfo[i].uiPMT_PID,astProgramInfo[i].aucProgramNanme,astProgramInfo[i].uiVideoPID,audioId);
		printf("%s\n",info);
		strncpy(freqInfo[i],info,strlen(info));
	}
	
}

int setProgramAndPidFilter()
{
	PROGRAM_INFO astProgramInfo[PROGRAM_MAX] = {0};
	int programCount = ParseTransportStream(fd,astProgramInfo);
	if(programCount > 0)
	{
		unsigned char pids[1024] = {0};
		int count = getProgramVideoAudioPids(astProgramInfo,programNumber,pids,programCount);
		Get_Pid_List(pids);
		if(pids != NULL && count > 6)
		{
			rtl2832_pid_filter_ctr(0);
			rtl2832_pid_filter(Pid_List,pid_count);
			PID_FILTER_FLAG = true;
			return SET_FILTER_SUCCESS;
		}
		else
			return SET_FILTER_FAIL;
	}
	else if(programCount == NO_TS_STREAM)
		return NO_TS_STREAM;
	else 
		return SET_FILTER_FAIL;	
} 

void HandleSignal(int signo)
{
	bTsThread = 0;
	PTHREAD_FLAG = false;
	printf("Thread quit!\n");
	return;
}

int Error_Handle()
{
	printf("Tuner Disconnect!\n");
	exit(-1);
}

static void close_tcp_sock()
{
	if(Tcp_Sock > 0)
	{	
		close(Tcp_Sock);
		Tcp_Sock = -1;
	}
	if(TCP_FD > 0)
	{
		close(TCP_FD);
		TCP_FD = -1;
	}
}

static void Clear()
{
	StopTsThread();	
	bTsThread = 0;
	close_tcp_sock();
	TCP_Flag = false;
	AcceptFlag = 0;
}

/*把USB读取到的数据通过UDP/TCP方式发送到指定客户端*/
static void  cb_img(struct libusb_transfer *transfer)
{
	if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
		fprintf(stderr, "img transfer status %d?\n", transfer->status);
		img_transfer[cImg] = NULL;
		memset(&imgbuf[0][0],0,1024*32);
		if(transfer->status == 5)
			Error_Handle(); //usb disconnect		
		return;
	}
	if (img_transfer[cImg] == NULL || libusb_submit_transfer(img_transfer[cImg]) < 0)
		return;
	
	int i = 0;
	int ret = 0;	
	int count = (transfer->actual_length)/1024;

	int SenddataSize = 0;

	memcpy(buf,&imgbuf[cImg][0],1024 * 32);

	if(AcceptFlag == 1 && Tcp_Sock > 0 && TCP_Flag)
	{
		while(i < count && bTsThread == 1)
		{
			ret = send(Tcp_Sock,buf + i * 1024,1024,0);
			if(ret < 0)
			{
				printf("###########################################\n");			
				printf("############send data fail!################\n");
				printf("###########################################\n");
				perror("the reason is:");
				unsigned char buf[20] = "send data fail";
				cImg = 0;			
				Clear();
				if(errno == EAGAIN || errno == EWOULDBLOCK)
				{
					sendFailMessage(buf,HB_Sock);
				}
				return;
			}

			SenddataSize = SenddataSize + ret;
			i++;
			errCount--;
		}	
	//	printf("send data size[%d]\n",SenddataSize);
	}
	else
	{
		if(writeCount <= 18 )
		{
			int size = fwrite(buf,1,imgSize,fd);
			if(size <= 0)
			{
				perror("write data fail!\n");
				return -1;
			}
			writeCount++;
			fflush(fd);
		//	printf("write data size[%d]\n",size);			
		}
		else
		{	
			fseek(fd,32768*5,SEEK_SET);
			StopTsThread();
			parseOkFlag = true;
		}
	}

	cImg++;
	if(cImg >= 1)
		cImg=0;	
	return ;
}

static int alloc_transfers(void)
{
	int i = 0;
	for(i = 0;i < 1;i++)
	{
		img_transfer[i] = libusb_alloc_transfer(0);
		if (!img_transfer[i])
			return -1;

		libusb_fill_bulk_transfer(img_transfer[i],dev, 0x81, &imgbuf[i][0],
			imgSize, cb_img, NULL, 0);
	}
	printf("alloc success!\n");
	return 0;
}


static int init_capture(void)
{
	int r = 0;
	int i = 0;
	for(i = 0;i < 1;i++)
	{
		r = libusb_submit_transfer(img_transfer[i]);
		if (r < 0) 
			return r;
	}
	printf("init captrue success!\n");
	return 0;
} 
void* thread_func(void *arg)
{
	int r = 0; 
	printf("Start udp stream！\n");	

	fd = fopen("/tmp/parse.ts", "wb+");
	if (!fd)
		return -1;

//	system("echo > /tmp/parse.ts");
	while(bTsThread)
	{		
		struct timeval tv = {1, 0 };
		r = libusb_handle_events_timeout(NULL, &tv);
		if (r < 0)
			break;
	}
	return NULL;
}

int StartTsThread(void)
{
	int t_arg = 100;
	int ret;
	pdev_handle = dev;
	writeCount = 0;
	
	if((!TCP_Flag && PID_FILTER_FLAG) || (programNumber <= 0 && !TCP_Flag))
	{
		ret = Create_Sock_Tcp();
		if(ret < 0)
			return -1;
		TCP_Flag = true;			
	}
	if(bTsThread == 0)
	{
		bTsThread=1;
		ret = set_pthread_policy(&attr3,80,&param);
		if(ret < 0)
			printf("set ts data pthread fail!\n");
		ret = pthread_create(&tid,&attr3,thread_func,&t_arg);
		if(ret != 0)
			printf("Create pthread error!\n");
		alloc_transfers();
		init_capture();
		printf("**TCP://@:8000**\n");
	}
	return 0;
}

int StopTsThread(void)
{
	bTsThread=0;
	writeCount = 0;
	
	parseOkFlag = false;
	PID_FILTER_FLAG = false;
	PTHREAD_FLAG = false;
	cImg = 0;
	if(&attr3 != NULL)
		pthread_attr_destroy(&attr3);
	memset(&imgbuf[0][0],0,1024*32);
	printf("udp stream thread Stop!\n");

	int i =0;

	for(i = 0;i < 1;i++)
	{
		if(img_transfer[i] != NULL)
		{
			int ret = libusb_cancel_transfer(img_transfer[i]);
			if (ret < 0) 
				return ret;	
		}
	}
	
	return 0;
}

