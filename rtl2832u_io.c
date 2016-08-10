

#include "rtl2832u_io.h"
#include "libusb.h"
#include"lusb0_usb.h"
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

#define ERROR_TRY_MAX_NUM	100

#define false 0

#define	DUMMY_PAGE		0x0a
#define	DUMMY_ADDR		0x01
#define READ_SIZE 63*1024
int fAbortReadMPEGThread = false;
//#pragma comment(lib,"ws2_32.lib")
//#pragma comment(lib,"libusb-1.0.lib")
//#pragma comment(lib,"libusb.lib")
const char* E_driver_version = "1.01" ; 
const char* E_driver_build_date = __DATE__ ; 
const char* E_driver_build_time = __TIME__ ;

// Device vendor and product id.

#if 1
#define MY_VID1 0x15F4
#define MY_PID1 0x0131
#define MY_PID2 0x0231

#define MY_VID2 0x15F4

#define MY_VID 0xea02
#define MY_PID 0x2488
#else
#define MY_VID 0x0bda
#define MY_PID 0x2832
#endif
libusb_device_handle *open_dev(void);
   libusb_device_handle *dev = NULL; /* the device handle */


#if 1
static  char tmp[4096];


pthread_mutex_t ghMutex = PTHREAD_MUTEX_INITIALIZER;
#if 0
DWORD WINAPI myProc(LPVOID lpv)
{
	// We have 32 buffers of 128K each for incoming data. That's 4MB or about 1 second's worth of a 30Mbit stream
	FILE *fp;
	int err=0;
	ULONG aLen = 0;
	int i=0;
	int dwStatus=0;
//	struct sockaddr_in from;
	struct sockaddr_in to;
	WSADATA wsaData;
	static SOCKET g_Socket = 0;
			
	WSAStartup(0x0202,&wsaData);	
	g_Socket = socket(AF_INET,SOCK_DGRAM,0);
	if(g_Socket == INVALID_SOCKET)
	{
		printf("socket failed!\n");
		WSACleanup();
		return 0;
	}

	to.sin_addr.s_addr = inet_addr("127.0.0.1");
	to.sin_family = AF_INET;
	to.sin_port = htons(1234);

	// Setup the asynchronous transfer buffers
	int nQueueSize = 32;
	void *request[32];
	PCHAR *buffers = new PCHAR[nQueueSize];

//	WaitForSingleObject(ghMutex,INFINITE);
	pthread_mutex_lock(&ghMutex);
	for(i=0;i<nQueueSize;i++)
	{
		buffers[i] = new CHAR[READ_SIZE];
		usb_bulk_setup_async(dev,&request[i],0x81);
		usb_submit_async(request[i], &buffers[i][0], READ_SIZE);
	}
	ReleaseMutex(ghMutex);
	i=0;

//	fp=fopen("D:\\test.ts","a+");

	while(fAbortReadMPEGThread)
	{	
//		WaitForSingleObject(ghMutex,INFINITE);
		pthread_mutex_lock(&ghMutex);

		aLen = usb_reap_async(request[i], 1000);
		ReleaseMutex(ghMutex);
	//	printf("aLen:%d   \n",aLen);
		if(aLen<=0)
		{
			Sleep(1);
			continue;
		}
		else
		{
			//Send to 
//			WaitForSingleObject(ghMutex,INFINITE);
			pthread_mutex_lock(&ghMutex);

			if(sendto(g_Socket,(const char *)&buffers[i][0],aLen,0,(struct sockaddr *)&to,sizeof(to)) == SOCKET_ERROR)
			{
				//printf("Error in sendto().\n");
			}
		//	fwrite((const char *)&buffers[i][0],1,aLen,fp);
			ReleaseMutex(ghMutex);
		}
//		WaitForSingleObject(ghMutex,INFINITE);
		pthread_mutex_lock(&ghMutex);

		usb_submit_async(request[i], &buffers[i][0], READ_SIZE);
		ReleaseMutex(ghMutex);
		i++;
		if(i==nQueueSize)
			i=0;
	}


	delete [] buffers;
	WSACleanup();

	return 0;
}
#endif
void print_endpoint(struct usb_endpoint_descriptor *endpoint)
{
	 printf(" bEndpointAddress: %02xh\n", endpoint->bEndpointAddress);
	 printf(" bmAttributes: %02xh\n", endpoint->bmAttributes);
	 printf(" wMaxPacketSize: %d\n", endpoint->wMaxPacketSize);
	 printf(" bInterval: %d\n", endpoint->bInterval);
	 printf(" bRefresh: %d\n", endpoint->bRefresh);
	 printf(" bSynchAddress: %d\n", endpoint->bSynchAddress);
}


void print_altsetting(struct usb_interface_descriptor *interface)
{
	 int i;

	 printf(" bInterfaceNumber: %d\n", interface->bInterfaceNumber);
	 printf(" bAlternateSetting: %d\n", interface->bAlternateSetting);
	 printf(" bNumEndpoints: %d\n", interface->bNumEndpoints);
	 printf(" bInterfaceClass: %d\n", interface->bInterfaceClass);
	 printf(" bInterfaceSubClass: %d\n", interface->bInterfaceSubClass);
	 printf(" bInterfaceProtocol: %d\n", interface->bInterfaceProtocol);
	 printf(" iInterface: %d\n", interface->iInterface);

	 for (i = 0; i < interface->bNumEndpoints; i++)
		print_endpoint( interface->endpoint);
}


void print_interface(struct usb_interface *interface)
{
	 int i;

	 for (i = 0; i < interface->num_altsetting; i++)
		 print_altsetting( interface->altsetting);
}


void print_configuration(struct usb_config_descriptor *config)
{
	 int i;

	 printf(" wTotalLength: %d\n", config->wTotalLength);
	 printf(" bNumInterfaces: %d\n", config->bNumInterfaces);
	 printf(" bConfigurationValue: %d\n", config->bConfigurationValue);
	 printf(" iConfiguration: %d\n", config->iConfiguration);
	 printf(" bmAttributes: %02xh\n", config->bmAttributes);
	 printf(" MaxPower: %d\n", config->MaxPower);

	 for (i = 0; i < config->bNumInterfaces; i++)
		print_interface( config->interface);
}
#if 1
libusb_device_handle *open_dev(void)
{
	struct libusb_device **devs;
	struct libusb_device *found = NULL;
	struct libusb_device *dev;
	struct libusb_device_handle *handle = NULL;
	size_t i = 0;
	int ret;
	if (libusb_get_device_list(NULL, &devs) < 0)
	{
		return NULL;
	}

	while ((dev = devs[i++]) != NULL) {
			struct libusb_device_descriptor desc;
			ret = libusb_get_device_descriptor(dev, &desc);

			printf("desc.idVendor[%04x]\ndesc.idProduct[%04x]\n",desc.idVendor , desc.idProduct );
			if ((desc.idVendor == MY_VID && desc.idProduct == MY_PID)||\
					(desc.idVendor == MY_VID1 && desc.idProduct == MY_PID1)||\
					(desc.idVendor == MY_VID2 && desc.idProduct == MY_PID2))
		    	{
		    		found = dev;
					break;

			}

		}
		 if (found) {
			ret = libusb_open(found, &handle);   ////////////
			if (ret < 0)
			{
				handle = NULL;
			}
		}
	out:

	//printf("\n\t test6_4.0       r;%d",r);
	libusb_free_device_list(devs, 1);
	return handle;

}
#endif

#if 0
usb_dev_handle *open_dev(void)
{
    struct usb_bus *bus;
    struct usb_device *dev;
	usb_dev_handle * hdev;
	 char string[256];
	 int ret,i;

    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if ((dev->descriptor.idVendor == MY_VID && dev->descriptor.idProduct == MY_PID)||\
				(dev->descriptor.idVendor == MY_VID1 && dev->descriptor.idProduct == MY_PID1)||\
				(dev->descriptor.idVendor == MY_VID2 && dev->descriptor.idProduct == MY_PID2))
            {
                hdev =  usb_open(dev);
			
			   if (hdev)
			   {
				if (dev->descriptor.iManufacturer)
				{
				 ret = usb_get_string_simple(hdev, dev->descriptor.iManufacturer, string, sizeof(string));
				 if (ret > 0)
					 printf("- Manufacturer : %s\n", string);
				 else
					 printf("- Unable to fetch manufacturer string\n");
				}
				if (dev->descriptor.iProduct)
				{
				 ret = usb_get_string_simple(hdev, dev->descriptor.iProduct, string, sizeof(string));
				 if (ret > 0)
					printf("- Product : %s\n", string);
				 else
					printf("- Unable to fetch product string\n");
				}
				if (dev->descriptor.iSerialNumber)
				{
				 ret = usb_get_string_simple(hdev, dev->descriptor.iSerialNumber, string, sizeof(string));
				 if (ret > 0)
					printf("- Serial Number: %s\n", string);
				 else
					printf("- Unable to fetch serial number string\n");
				}

				if (!dev->config)
				{
				 printf(" Couldn't retrieve descriptors\n");
				}

				for (i = 0; i < dev->descriptor.bNumConfigurations; i++)
						print_configuration( dev->config);
			   }

				return hdev;
            }
        }
    }
    return NULL;
}
#endif

int CloseUSBPort(void)
{
	 if (dev)
    {
        libusb_close(dev);
    }
	return 0;
} 

int  usbi2cinit()
{
	int err = 0;
	printf ("CYUSBI2C.dll Version %s, Built %s %s\n",E_driver_version, E_driver_build_date,E_driver_build_time);

	if(dev)
		libusb_close(dev);

    libusb_init(NULL); /* initialize the library */
//    usb_find_busses(); /* find all busses */   //maybe
//    usb_find_devices(); /* find all connected devices */   //maybe





    if (!(dev = open_dev()))
    {
 //       printf("error opening device: \n%s\n", usb_strerror());
        return -1;
    }
    else
    {
        printf("success: device %04X:%04X opened\n", MY_VID, MY_PID);
    }

//	err = usb_set_configuration(dev,1);
//	printf("usb_set_configuration;%d   \n",err);

	err = libusb_claim_interface(dev,0);
	printf("libusb_claim_interface;%d   \n",err);
	err = libusb_set_interface_alt_setting(dev,0,0);
	printf("libusb_set_interface_alt_setting;%d   \n",err);

//	err = usb_release_interface(dev,0);
//	printf("usb_release_interface;%d   \n",err);


//	ghMutex = OpenMutex(SYNCHRONIZE,FALSE,"MyTUNERMutex");
//	if(!ghMutex)
//	{	
//		ghMutex = CreateMutex(NULL,TRUE,"MyTUNERMutex")	;
//		ReleaseMutex(ghMutex);
//	}

	
		pthread_mutexattr_t mattr;

		pthread_mutex_init(&ghMutex, &mattr);
		pthread_mutexattr_init(&mattr);

//		pthread_mutex_unlock(&ghMutex);

	return err;
}

static int bTsThread = 0;

#if 0

FILE *fd;

void thread_func(void *arg)
{
	int *val = arg;
	//int uSize = 1024*32;
	printf("Hi,I'm a thread!\n");
	int len;
	//unsigned char in_data_buf[32][uSize];
	
	int i=0,err;
	int transffered_bytes; 

	i=0;
	int r = 0;
	fd = fopen("ts.ts", "a+b");
	if (!fd)
		return -1;
     
	while(bTsThread)
	{		
		struct timeval tv = { 1, 0 };
		r = libusb_handle_events_timeout(NULL, &tv);
		if (r < 0) {
			break;
		}
	}
	
fclose(fd);
}

int StartTsThread(void)
{
#if 0
	if(bTsThread==0)
	{
		bTsThread=1;
		HANDLE hThread;
		DWORD dwThreadID;
		fAbortReadMPEGThread = TRUE;
		hThread = CreateThread(NULL, 0, myProc, (LPVOID)0, CREATE_SUSPENDED, &dwThreadID);
		SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
		ResumeThread(hThread);
		CloseHandle(hThread);

		printf("**UDP://@127.0.0.1:1234 **\n");
	}
	return 0;
	#endif
	pthread_t tid;
	int t_arg=100;
	int ret;
	if(bTsThread==0)
	{
		bTsThread=1;
		ret = pthread_create(&tid,NULL,thread_func,&t_arg);
		if(ret!=0)
			printf("Create pthread error!\n");
		alloc_transfers();
		init_capture();
		printf("**UDP://@127.0.0.1:1234 **\n");
	}
	return 0;

	
}


int StopTsThread(void)
{
	bTsThread=0;
	fAbortReadMPEGThread = false;
	Sleep(1000);
	return 0;
}
#endif

void platform_wait(unsigned long nMinDelayTime)
{
	// The unit of Sleep() waiting time is millisecond.
#if 0
	unsigned long usec;
	do {
		usec = (nMinDelayTime > 8000) ? 8000 : nMinDelayTime;
		//ning 
		Sleep(usec);
		nMinDelayTime -= usec;
	} while (nMinDelayTime > 0);
#else
	struct timeval time;
	time.tv_sec = 0;
	time.tv_usec = nMinDelayTime * 1000;
	select(0,NULL,NULL,NULL,&time);

//	usleep(nMinDelayTime);
#endif
	return;
	
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		remote control 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int 
read_rc_register(
	unsigned short	offset,
	unsigned char*	data,
	unsigned short	bytelength)
{
	int ret = -1;
// WaitForSingleObject(ghMutex,INFINITE);
	pthread_mutex_lock(&ghMutex);

    ret = usb_control_msg(dev,								/* pointer to device */
            SKEL_VENDOR_IN,										/* USB message request type value */
			0,											/* USB message request value */
            offset,											/* USB message value */
            0x0201,											/* USB message index value */
            (char*)data,											/* pointer to the receive buffer */
            bytelength,										/* length of the buffer */
            DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */
//ReleaseMutex(ghMutex);
	pthread_mutex_unlock(&ghMutex);

    if (ret != bytelength)
	{
//		deb_info(" error try rc read register %s: offset=0x%x, error code=0x%x !\n", __FUNCTION__, offset, ret);
		return 1;
    }

	return 0; 
}



int 
write_rc_register(
	unsigned short	offset,
	unsigned char*	data,
	unsigned short	bytelength)
{
	int ret = -1;
	unsigned char try_num;

	try_num = 0;	
	error_write_again:
	try_num++;	
// WaitForSingleObject(ghMutex,INFINITE);
		pthread_mutex_lock(&ghMutex);

        ret = usb_control_msg(dev,								/* pointer to device */
                SKEL_VENDOR_OUT,									/* USB message request type value */
				0,											/* USB message request value */
                offset,											/* USB message value */
                0x0211,											/* USB message index value */
                (char *)data,											/* pointer to the receive buffer */
                bytelength,										/* length of the buffer */
                DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */
//ReleaseMutex(ghMutex);
		pthread_mutex_unlock(&ghMutex);

        if (ret != bytelength)
		{
//			deb_info("error try rc write register  = %d, %s: offset=0x%x, error code=0x%x !\n",try_num ,__FUNCTION__, offset, ret);

			if( try_num > ERROR_TRY_MAX_NUM )	
				goto error;
			else				
				goto error_write_again;
       }

	return 0;
error:
	return 1;
 }


int 
read_rc_char_bytes(
	RegType	type,
	unsigned short	byte_addr,
	unsigned char*	buf,
	unsigned short	byte_num)
{
	int ret = 1;

	if( byte_num != 1 && byte_num !=2 && byte_num !=4 && byte_num != 0x80)
	{
//		deb_info("error!! %s: length = %d \n", __FUNCTION__, byte_num);
		return 1;
	}


//	if( mutex_lock_interruptible(&dib->usb_mutex) )	return 1;
	
	if (type == RTD2832U_RC )	
		ret = read_rc_register( byte_addr , buf , byte_num);
	else
	{
//		deb_info("error!! %s: erroe register type \n", __FUNCTION__);
		return 1;
	}			
//	mutex_unlock(&dib->usb_mutex);
	return ret;
	
}



int 
write_rc_char_bytes(
	RegType	type,
	unsigned short	byte_addr,
	unsigned char*	buf,
	unsigned short	byte_num)
{
	int ret = 1;

	if( byte_num != 1 && byte_num !=2 && byte_num !=4 && byte_num !=0x80)
	{
//		deb_info("error!! %s: length = %d \n", __FUNCTION__, byte_num);
		return 1;
	}

//	if( mutex_lock_interruptible(&dib->usb_mutex) )	return 1;

	if (type == RTD2832U_RC )	
		ret = write_rc_register( byte_addr , buf , byte_num);
	else
	{
//		deb_info("error!! %s: erroe register type \n", __FUNCTION__);
		ret=1;
	}	
//	mutex_unlock(&dib->usb_mutex);	

	return ret;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 int  
read_usb_register(
	unsigned short	offset,
	unsigned char*	data,
	unsigned short	bytelength)
{
	int ret = -1;
// WaitForSingleObject(ghMutex,INFINITE);
	pthread_mutex_lock(&ghMutex);

    ret = usb_control_msg(dev,								/* pointer to device */
            SKEL_VENDOR_IN,										/* USB message request type value */
			0,														/* USB message request value */
            (USB_BASE_ADDRESS<<8) + offset,						/* USB message value */
            0x0100,													/* USB message index value */
           (char *) data,													/* pointer to the receive buffer */
            bytelength,												/* length of the buffer */
            DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */
//ReleaseMutex(ghMutex);
		pthread_mutex_unlock(&ghMutex);

        if (ret != bytelength)
		{
			printf("#### offset=0x%x, error code=0x%x !\n", offset, ret);
			return 1;
       }

	return 0; 
}



 int 
write_usb_register(
	unsigned short	offset,
	unsigned char*	data,
	unsigned short	bytelength)
{
	int ret = -1;
	unsigned char try_num;

	try_num = 0;	
	error_write_again:
	try_num++;	
// WaitForSingleObject(ghMutex,INFINITE);
 pthread_mutex_lock(&ghMutex);
        ret = usb_control_msg(dev,								/* pointer to device */
                SKEL_VENDOR_OUT,										/* USB message request type value */
				0,														/* USB message request value */
                (USB_BASE_ADDRESS<<8) + offset,						/* USB message value */
                0x0110,													/* USB message index value */
                (char *)data,													/* pointer to the receive buffer */
                bytelength,												/* length of the buffer */
                DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */
//ReleaseMutex(ghMutex);
		pthread_mutex_unlock(&ghMutex);

        if (ret != bytelength)
		{
	//		deb_info("error try = %d, %s: offset=0x%x, error code=0x%x !\n",try_num ,__FUNCTION__, offset, ret);

			if( try_num > ERROR_TRY_MAX_NUM )	
				goto error;
			else			
				goto error_write_again;
       }

	return 0;
error:
	return 1;
 }




 int 
read_sys_register(
	unsigned short	offset,
	unsigned char*	data,
	unsigned short	bytelength)
{
	int ret = -1;
// WaitForSingleObject(ghMutex,INFINITE);
		pthread_mutex_lock(&ghMutex);

        ret = usb_control_msg(dev,								/* pointer to device */
                SKEL_VENDOR_IN,										/* USB message request type value */
				0,														/* USB message request value */
                (SYS_BASE_ADDRESS<<8) + offset,						/* USB message value */
                0x0200,													/* USB message index value */
               (char *) data,													/* pointer to the receive buffer */
                bytelength,												/* length of the buffer */
                DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */
//		ReleaseMutex(ghMutex);	
		pthread_mutex_unlock(&ghMutex);

        if (ret != bytelength)
		{
			printf(" offset=0x%x, error code=0x%x !\n", offset, ret);
			return 1;
       }

	return 0; 

  }


 int 
write_sys_register(
	unsigned short	offset,
	unsigned char*	data,
	unsigned short	bytelength)
{ 
	int ret = -1;
	unsigned char try_num;

	try_num = 0;	
	error_write_again:	
	try_num++;	
//WaitForSingleObject(ghMutex,INFINITE);
		pthread_mutex_lock(&ghMutex);

        ret = usb_control_msg(dev,								/* pointer to device */
                SKEL_VENDOR_OUT,										/* USB message request type value */
				 0,														/* USB message request value */
                (SYS_BASE_ADDRESS<<8) + offset,						/* USB message value */
                0x0210,													/* USB message index value */
               (char *) data,													/* pointer to the receive buffer */
                bytelength,												/* length of the buffer */
                DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */
//		ReleaseMutex(ghMutex);
		pthread_mutex_unlock(&ghMutex);

        if (ret != bytelength)
		{
//			deb_info(" error try= %d, %s: offset=0x%x, error code=0x%x !\n",try_num, __FUNCTION__, offset, ret);
			if( try_num > ERROR_TRY_MAX_NUM )
				goto error;
			else	
				goto error_write_again;	
        }

	return 0;
error:
	return 1;
 }




int 
read_demod_register(
	unsigned char			demod_device_addr,	
	unsigned char 		page,
	unsigned char 		offset,
	unsigned char*		data,
	unsigned short		bytelength)
{
	int ret = -1;
	int i;
	char	tmp;
//WaitForSingleObject(ghMutex,INFINITE);
	pthread_mutex_lock(&ghMutex);

	ret = usb_control_msg(dev,									/* pointer to device */        													
        SKEL_VENDOR_IN,											/* USB message request type value */
		0,														/* USB message request value */
        demod_device_addr + (offset<<8),						/* USB message value */
        (0x0000 + page),										/* USB message index value */ 
        (char *) data,											/* pointer to the receive buffer */
        bytelength,												/* length of the buffer */

        DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */
//	ReleaseMutex(ghMutex);
pthread_mutex_unlock(&ghMutex);

#if 0
	   usb_control_msg(dev,											/* pointer to device */
        SKEL_VENDOR_IN,										/* USB message request type value */
		 0,														/* USB message request value */  
        (0x0000 + DUMMY_PAGE),										/* USB message index value */
		RTL2832_DEMOD_ADDR + (DUMMY_ADDR<<8),						/* USB message value */
        &tmp,													/* pointer to the receive buffer */
        LEN_1_BYTE,												/* length of the buffer */
        DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */

#endif	
	//	deb_info("%s[1345]: ret=%d, DA=0x%x, len=%d, page=%d, offset=0x%x, data=(", __FUNCTION__, ret, demod_device_addr, bytelength,page, offset);
//		for(i = 0; i < bytelength; i++)
//			deb_info("0x%x,", data[i]);
//		deb_info(")\n");
			
        if (ret != bytelength)
		{
			printf("error!! : ret=%d, DA=0x%x, len=%d, page=%d, offset=0x%x, data=(", ret, demod_device_addr, bytelength,page, offset);
			for(i = 0; i < bytelength; i++)
				printf("0x%x,", data[i]);
			printf(")\n");
			
			goto error;
       }

	return 0;  

error:
	return 1;
}




int 
write_demod_register(
	unsigned char			demod_device_addr,		
	unsigned char			page,
	unsigned char			offset,
	unsigned char			*data,
	unsigned short		bytelength)
{
	int ret = -1;
	unsigned char  i, try_num;
	 char	tmp;	

	try_num = 0;
	

error_write_again:	
	try_num++;

//	if( mutex_lock_interruptible(&dib->usb_mutex) )	goto error;
//WaitForSingleObject(ghMutex,INFINITE);
		pthread_mutex_lock(&ghMutex);

        ret = usb_control_msg(dev,								/* pointer to device */     														
                SKEL_VENDOR_OUT,										/* USB message request type value */
				0,														/* USB message request value */
				demod_device_addr + (offset<<8),						/* USB message value */
                (0x0010 + page),										/* USB message index value */	
                (char *)data,													/* pointer to the receive buffer */
                bytelength,												/* length of the buffer */
                DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */
//ReleaseMutex(ghMutex);
	pthread_mutex_unlock(&ghMutex);

	#if 0
				usb_control_msg(dev,								/* pointer to device */            
				SKEL_VENDOR_IN,										/* USB message request type value */
				0,														/* USB message request value */
				(0x0000 + DUMMY_PAGE),										/* USB message index value */
				 RTL2832_DEMOD_ADDR + (DUMMY_ADDR<<8),						/* USB message value */
				&tmp,													/* pointer to the receive buffer */
				LEN_1_BYTE,												/* length of the buffer */
				DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */

	#endif
//	mutex_unlock(&dib->usb_mutex);

//		deb_info("%s: ret=%d, DA=0x%x, len=%d, page=%d, offset=0x%x, data=(", __FUNCTION__, ret, demod_device_addr, bytelength, page,offset);
//		for(i = 0; i < bytelength; i++)
//			deb_info("0x%x,", data[i]);
//		deb_info(")\n");


     if (ret != bytelength)
	{
		printf("*error try = %d! ret=%d, DA=0x%x, len=%d, page=%d, offset=0x%x, data=(",try_num , ret, demod_device_addr, bytelength,page,offset);
		for(i = 0; i < bytelength; i++)
			printf("0x%x,", data[i]);
		printf(")\n");
	
		if( try_num > ERROR_TRY_MAX_NUM )	
			goto error;
		else				
			goto error_write_again;
    }

	return 0;

error:
	return 1;
 }


int 
read_rtl2832_tuner_register(
	unsigned char			device_address,
	unsigned char			offset,
	unsigned char			*data,
	unsigned short		bytelength)
{
	int ret = -1;
 	int i;
	 char data_tmp[128];	


//	if( mutex_lock_interruptible(&dib->usb_mutex) )	goto error;
//	WaitForSingleObject(ghMutex,INFINITE);
		pthread_mutex_lock(&ghMutex);

        ret = usb_control_msg(dev,								/* pointer to device */
                
                SKEL_VENDOR_IN,										/* USB message request type value */
				0,														/* USB message request value */
                device_address+(offset<<8),							/* USB message value */
                0x0300,													/* USB message index value */
                data_tmp,												/* pointer to the receive buffer */
                bytelength,												/* length of the buffer */
                DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */
//		ReleaseMutex(ghMutex);
pthread_mutex_unlock(&ghMutex);

//	mutex_unlock(&dib->usb_mutex);

//		deb_info("%s: ret=%d, DA=0x%x, len=%d, offset=0x%x, data=(", __FUNCTION__, ret, device_address, bytelength,offset);
//		for(i = 0; i < bytelength; i++)
//			deb_info("0x%x,", data_tmp[i]);
//		deb_info(")\n");
			
        if (ret != bytelength)
 		{
			printf("error!! : ret=%d, DA=0x%x, len=%d, offset=0x%x, data=(", ret, device_address, bytelength,offset);
			for(i = 0; i < bytelength; i++)
				printf("0x%x,", data_tmp[i]);
			printf(")\n");
			
			goto error;
        }

	memcpy(data,data_tmp,bytelength);

	return 0;
	
error:
	return 1;   
	
 
}

int  write_rtl2832_tuner_register(
	unsigned char			device_address,
	unsigned char			offset,
	unsigned char			*data,
	unsigned short		bytelength)
{
	int ret = -1;
	unsigned char  i, try_num;

	try_num = 0;	
	error_write_again:	
	try_num++;

//	if( mutex_lock_interruptible(&dib->usb_mutex) )	goto error;
// WaitForSingleObject(ghMutex,INFINITE);
		pthread_mutex_lock(&ghMutex);

        ret = usb_control_msg(dev,								/* pointer to device */
               
                SKEL_VENDOR_OUT,										/* USB message request type value */
				 0,														/* USB message request value */
                device_address+(offset<<8),							/* USB message value */
                0x0310,													/* USB message index value */
                (char *)data,													/* pointer to the receive buffer */
                bytelength,												/* length of the buffer */
                DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */
//ReleaseMutex(ghMutex);
pthread_mutex_unlock(&ghMutex);

//	mutex_unlock(&dib->usb_mutex);

//		deb_info("%s: ret=%d, DA=0x%x, len=%d, offset=0x%x, data=(", __FUNCTION__, ret, device_address, bytelength, offset);
//		for(i = 0; i < bytelength; i++)
//			deb_info("0x%x,", data[i]);
//		deb_info(")\n");

			
        if (ret != bytelength)
	{
//		deb_info("error try= %d!! %s: ret=%d, DA=0x%x, len=%d, offset=0x%x, data=(",try_num, __FUNCTION__, ret, device_address, bytelength, offset);
		for(i = 0; i < bytelength; i++)
//			deb_info("0x%x,", data[i]);
//		deb_info(")\n");
		
		if( try_num > ERROR_TRY_MAX_NUM )	goto error;
		else				goto error_write_again;
       }

	return 0;

error:
	return 1;
 }




int 
read_rtl2832_stdi2c(
	unsigned short			dev_i2c_addr,
	unsigned char*			data,
	unsigned short			bytelength)
{
	int i;
	int ret = -1;
	unsigned char  try_num;
	 char data_tmp[128];	

	try_num = 0;	
error_write_again:		
	try_num ++;	
        

	if(bytelength >= 128)
	{
//		deb_info("%s error bytelength >=128  \n", __FUNCTION__);
		goto error;
	}

//	if( mutex_lock_interruptible(&dib->usb_mutex) )	goto error;
//	WaitForSingleObject(ghMutex,INFINITE);
	pthread_mutex_lock(&ghMutex);

	ret = usb_control_msg(dev,								/* pointer to device */
		
		SKEL_VENDOR_IN,											/* USB message request type value */
		0,														/* USB message request value */
		dev_i2c_addr,											/* USB message value */
		0x0600,													/* USB message index value */
		data_tmp,												/* pointer to the receive buffer */
		bytelength,												/* length of the buffer */
		DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */
//ReleaseMutex(ghMutex);
pthread_mutex_unlock(&ghMutex);

//	mutex_unlock(&dib->usb_mutex);
 
	if (ret != bytelength)
	{
#if 0
		if(ret == -9)
		{
		//	ResetI2C();
		}
		else
		{

		}
#endif
		printf("********************************\n");		
		printf("error try= %d!! : ret=%d, DA=0x%x, len=%d, data=(",try_num, ret, dev_i2c_addr, bytelength);
		printf("********************************\n");
		for(i = 0; i < bytelength; i++)
			printf("0x%x,", data_tmp[i]);
		printf(")\n");		
		
		if( try_num > ERROR_TRY_MAX_NUM )	
			goto error;
		else
			goto error_write_again;
	}

 	memcpy(data,data_tmp,bytelength);

	return 0;  
error: 
	return 1;

}


int 
write_rtl2832_stdi2c(
	unsigned short			dev_i2c_addr,
	unsigned char*			data,
	unsigned short			bytelength)
{
	int i;
	int ret = -1;  
	unsigned char  try_num;

	try_num = 0;	
error_write_again:		
	try_num ++;	

//	if( mutex_lock_interruptible(&dib->usb_mutex) )	goto error;
//WaitForSingleObject(ghMutex,INFINITE);
	pthread_mutex_lock(&ghMutex);

	ret = usb_control_msg(dev,								/* pointer to device */
	
		SKEL_VENDOR_OUT,										/* USB message request type value */
			0,														/* USB message request value */
		dev_i2c_addr,											/* USB message value */
		0x0610,													/* USB message index value */
	(char *)	data,													/* pointer to the receive buffer */
		bytelength,												/* length of the buffer */
		DIBUSB_I2C_TIMEOUT);									/* time to wait for the message to complete before timing out */
//	ReleaseMutex(ghMutex);
pthread_mutex_unlock(&ghMutex);

//	mutex_unlock(&dib->usb_mutex);
 if (ret != bytelength)

	{
		if(ret==-9)
		{
			printf("###########error! reset i2c and try again!######\n");
			ResetI2C();
		}
		else
		{
			printf("###############################################\n");
			printf("error try= %d!! ret=%d, DA=0x%x, len=%d, data=(",try_num, ret, dev_i2c_addr, bytelength);
			printf("###############################################\n");
			for(i = 0; i < bytelength; i++)
				printf("0x%x,", data[i]);

			printf(")\n");
		}

		if( try_num > ERROR_TRY_MAX_NUM )	goto error;

		else				goto error_write_again;		



	}

 
	return 0;
	
error:
	return 1;  
	
}


//3for return PUCHAR value
int 
read_usb_sys_char_bytes(
	RegType	type,
	unsigned short	byte_addr,
	unsigned char*	buf,
	unsigned short	byte_num)
{
	int ret = 1;

	if( byte_num != 1 && byte_num !=2 && byte_num !=4)
	{
		printf("error!!  length = %d \n", byte_num);
		return 1;
	}


//	if( mutex_lock_interruptible(&dib->usb_mutex) )	return 1;
//	pthread_mutex_unlock(&ghMutex);
	if( type == RTD2832U_USB )
	{
		ret = read_usb_register( byte_addr , buf , byte_num);
	}
	else if ( type == RTD2832U_SYS )
	{
		ret = read_sys_register(  byte_addr , buf , byte_num);
	}
//	pthread_mutex_unlock(&ghMutex);
	//mutex_unlock(&dib->usb_mutex);

	return ret;
	
}



int 
write_usb_sys_char_bytes(
	RegType	type,
	unsigned short	byte_addr,
	unsigned char*	buf,
	unsigned short	byte_num)
{
	int ret = 1;

	if( byte_num != 1 && byte_num !=2 && byte_num !=4)
	{
		printf("error!!  length = %d \n", byte_num);
		return 1;
	}

//	if( mutex_lock_interruptible(&dib->usb_mutex) )	return 1;	
//	pthread_mutex_lock(&ghMutex);

	if( type == RTD2832U_USB )
	{
		ret = write_usb_register(  byte_addr , buf , byte_num);
	}
	else if ( type == RTD2832U_SYS )
	{
		ret = write_sys_register(  byte_addr , buf , byte_num);
	}
//	pthread_mutex_unlock(&ghMutex);
//	mutex_unlock(&dib->usb_mutex);	
	
	return ret;
	
}


//3for return INT value
int 
read_usb_sys_int_bytes(
	RegType	type,
	unsigned short	byte_addr,
	unsigned short	n_bytes,
	int*	p_val)
{
	int	i;
	unsigned char	val[LEN_4_BYTE];
	int	nbit_shift; 

	*p_val= 0;

	if (read_usb_sys_char_bytes(  type, byte_addr, val , n_bytes)) goto error;

	for (i= 0; i< n_bytes; i++)
	{				
		nbit_shift = i<<3 ;
		*p_val = *p_val + (val[i]<<nbit_shift );
       }

	return 0;
error:
	return 1;
			
}



int 
write_usb_sys_int_bytes(
	RegType	type,
	unsigned short	byte_addr,
	unsigned short	n_bytes,
	int	val)
{
	int	i;
	int	nbit_shift;
	unsigned char	u8_val[LEN_4_BYTE];		

	for (i= n_bytes- 1; i>= 0; i--)
	{
		nbit_shift= i << 3;		
		u8_val[i] = (val>> nbit_shift) & 0xff;
    	}

	if( write_usb_sys_char_bytes(  type , byte_addr, u8_val , n_bytes) ) goto error;			

	return 0;
error:
	return 1;
}



int 
write_rtl2836_demod_register(
	unsigned char			demod_device_addr,		
	unsigned char			page,
	unsigned char			offset,
	unsigned char			*data,
	unsigned short		bytelength)
{
	int i;
	unsigned char           datatmp;
	int                            try_num;
	switch(page)
	{
		//3 R/W regitser Once R/W "ONE BYTE"
		case PAGE_0:
		case PAGE_1:
		case PAGE_2:
		case PAGE_3:
		case PAGE_4:
			for(i=0; i<bytelength; i++)
			{
				try_num = 0;

label_write:
				if(write_demod_register( demod_device_addr, page,  offset+i,  data+i, LEN_1_BYTE))
					goto error;

label_read:
				if(try_num >= 4)
					goto error;

				if(read_demod_register( demod_device_addr, page,  offset+i,  &datatmp, LEN_1_BYTE))
				{
					try_num++;
//					deb_info("%s fail read\n", __FUNCTION__);
					goto label_read;
				}
				
				if(datatmp != data[i])
				{
				       try_num++;
//					deb_info("%s read != write\n", __FUNCTION__);   
					goto label_write;
				}								
			}
			break;

		default:
			goto error;
			break;
	}

	return 0;
	
error:
	return 1;
}



int 
read_rtl2836_demod_register(
	unsigned char			demod_device_addr,	
	unsigned char 		page,
	unsigned char 		offset,
	unsigned char*		data,
	unsigned short		bytelength)
{

       int i;
	unsigned char  tmp;
		
	switch(page)
	{
		// R/W regitser Once R/W "ONE BYTE"
		case PAGE_0:
		case PAGE_1:
		case PAGE_2:
		case PAGE_3:
		case PAGE_4:
			for(i=0; i<bytelength; i++)
			{
				if(read_demod_register( demod_device_addr, page,  offset+i,  data+i, LEN_1_BYTE))
					goto error;
			}
			break;
				
       	case PAGE_6:
		case PAGE_7:
		case PAGE_8:
		case PAGE_9:
			if(read_demod_register( demod_device_addr, page,  offset,  data, bytelength)) 
				goto error;
			break;	

              case PAGE_5:
		       if(read_demod_register( demod_device_addr, page,  offset,  data, bytelength)) 
				goto error;

			if(read_demod_register( demod_device_addr, PAGE_0, 0x00, &tmp, LEN_1_BYTE))//read page 0
				goto error;
			
		       break;

		default:
			goto error;
			break;
	}

	return 0;
	
error:
	return 1;

}


#endif






