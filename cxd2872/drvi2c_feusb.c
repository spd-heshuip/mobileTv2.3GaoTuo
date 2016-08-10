/*------------------------------------------------------------------------------
  Copyright 2009-2013 Sony Corporation

  Last Updated  : 2013/02/22
  File Revision : 1.0.2.0
------------------------------------------------------------------------------*/

#include "drvi2c_feusb.h"
//#include "i2c_feusb.h"
extern unsigned char DemMode,TunerHV;
extern unsigned char cxd2872adr;

sony_result_t drvi2c_feusb_Read(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t* pData,
                  uint32_t size, uint8_t mode)
{
      // drvi2c_feusb_t *pDrvI2c = NULL;
printf("drvi2c_feusb_Read.\n");
	   int result=0;
	  unsigned char buf[10];
	   SONY_TRACE_I2C_ENTER("drvi2c_Read");
	
	   SONY_ARG_UNUSED(mode);
	
	//	 if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
	
	 //  pDrvI2c = (drvi2c_t*)(pI2c->user);
	 //  if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
#if 0
	if(DemMode==38){//MN88436
		int sub;
	   *pData=0;
			unsigned char MN88436_adr = 0x30;
			if(Get_MN88436_DemodType()==2)
			{
				if(TunerHV&1)
					MN88436_adr = 0x32;
				else 
					MN88436_adr = 0x30;
			}
			else if(Get_MN88436_DemodType()==1)
			{
				MN88436_adr = 0x32;
			}
			else
			{
				MN88436_adr = 0x30;
			}
	
			buf[0]=0x15;
			buf[1]=0x53;
			I2cWrite(MN88436_adr,buf,2,1);
			
			buf[0]=0x17;
			buf[1]=0x00;
			I2cWrite(MN88436_adr,buf,2,1);
	
			sub = 0x20;
			sub<<=8;
			sub+=cxd2872adr+1;
			result = I2cRead16_NoStop(MN88436_adr,sub,pData,size);
			if(result)
			printf("MN88436 Read cxd2872 Tuner err	 %x\n",0xc0);
	
		}
	else
#endif
	{
#if 1
	   result = read_rtl2832_stdi2c(deviceAddress, pData, size);
		if(result)
			printf("drvi2c_feusb_Read Err\n");
#else

{

	
	//result = I2cRead16_NoStop(pI2c->gwAddress, sub_addr,pData, size);
   result = read_rtl2832_stdi2c(deviceAddress,pData,size);

printf("****************R  pData=>%x \n",pData[0]);

}
#endif
		}
		
	//else
		//printf("drvi2c_feusb_Read Success\n");
		  switch (result) {
		   case 0:
			   SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
			   break;
		   case 1:
			   SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
			   break;
		   default:
			SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
			   break;
	   }

}

sony_result_t drvi2c_feusb_Write(sony_i2c_t* pI2c, uint8_t deviceAddress, const uint8_t * pData,
                  uint32_t size, uint8_t mode)
{
     // drvi2c_feusb_t *pDrvI2c = NULL;
	  int result = 0;
	 unsigned char buf[10];
	  SONY_TRACE_I2C_ENTER("drvi2c_Write");
	
	  SONY_ARG_UNUSED(mode);
	
	//	if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
	
	//	pDrvI2c = (drvi2c_t*)(pI2c->user);
	//	if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
#if 0
	 if(DemMode==38){//MN88436

		unsigned char MN88436_adr = 0x30;
		if(Get_MN88436_DemodType()==2)
		{
			if(TunerHV&1)
				MN88436_adr = 0x32;
			else 
				MN88436_adr = 0x30;
		}
		else if(Get_MN88436_DemodType()==1)
		{
			MN88436_adr = 0x32;
		}
		else
		{
			MN88436_adr = 0x30;
		}
	
		buf[0]=0x15;
		buf[1]=0x53;
		I2cWrite(MN88436_adr,buf,2,1);
		
		buf[0]=0x17;
		buf[1]=0x00;
		I2cWrite(MN88436_adr,buf,2,1);
		
			if(size==1){
				buf[0]=MN88436_adr;
				buf[1]=0x20;
				buf[2]=cxd2872adr;
				buf[3]=pData[0];
				result = I2cWrite(buf[0],&buf[1],3,1);
				if(result)
					printf("MN88472 I2C cxd2872 write Error.\n");
			}
			else
			{	//
				buf[0] = MN88436_adr;
				buf[1] = 0x20;
				buf[2] = cxd2872adr;
				memcpy(&buf[3],pData,size);
				result = I2cWrite(buf[0],&buf[1],size+2,1);
				if(result)
					printf("MN88436 write cxd2872 err   %x\n",0xc0);
			}
	}

	else
#endif
	{



#if 1
		printf("#######start to write rtl2832 i2c...#####\n");
	  	result = write_rtl2832_stdi2c(deviceAddress, pData, size,1);
		if(result)
			printf("drvi2c_feusb_Write Err\n");
		printf("#######write rtl2832 i2c success!######\n");
#else

{
unsigned char data[128];
	data[0]=pI2c->gwSub;
    data[1]=deviceAddress;
    memcpy(&data[2],pData,size);
	/*
		The Write gateway function pointer should:
		\verbatim {S} [Addr] [GW Sub] [TunerAddr] [Data0] [Data1] ... [DataN] {P} \endverbatim

	*/
printf("*******************W  pData=>%x \n",pData[0]);
    result = write_rtl2832_stdi2c(data[1],&data[2],size);
}
#endif


		}
		//else
		//printf("drvi2c_feusb_Write Success\n");
		 switch (result) {
		  case 0:
			  SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
			  break;
		  case 1:
			  SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
			  break;
		  default:
		   SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
			  break;
	  }

}

/* For gateway read access */
sony_result_t drvi2c_feusb_ReadGw(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t* pData,
                  uint32_t size, uint8_t mode)
{
     //   drvi2c_feusb_t *pDrvI2c = NULL;
		int result;
		unsigned int sub_addr;
		SONY_TRACE_I2C_ENTER("drvi2c_ReadGw");
	
		SONY_ARG_UNUSED(mode);
	
	 // if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
	
	  // pDrvI2c = (drvi2c_t*)(pI2c->user);
	  // if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
#if 0
		sub_addr = 0x09;//pI2c->gwSub;;
		sub_addr<<=8;
		sub_addr+=deviceAddress|1;
		result = I2cRead16_NoStop(pI2c->gwAddress, sub_addr,pData, size);
		if(result)
		printf("drvi2c_feusb_ReadGw Err\n");
#else


{

	
	//result = I2cRead16_NoStop(pI2c->gwAddress, sub_addr,pData, size);
   result = read_rtl2832_stdi2c(deviceAddress,pData,size);

//printf("****************R  pData=>%x \n",pData[0]);

}

#endif
		//else
		//printf("drvi2c_feusb_ReadGw Success\n");


		//	printf("drvi2c_WriteGw deviceAddress:%x pI2c->gwAddress:%x pI2c->gwSub:%x\n",deviceAddress,pI2c->gwAddress,pI2c->gwSub);
		switch (result) {
			case 0:
				SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
				break;
			case 1:
				SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
				break;
			default:
			 SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
				break;
		}
	/*
		switch(i2c_ReadGw(pDrvI2c->handle, deviceAddress, pData, size,
								 pI2c->gwAddress, pI2c->gwSub))
		{
		case I2C_OK:
			SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
		case I2C_ERROR_ACK:
		case I2C_ERROR_ACCESS:
			SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
		default:
			SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
		}
		*/

}

/* For gateway write access */
sony_result_t drvi2c_feusb_WriteGw(sony_i2c_t* pI2c, uint8_t deviceAddress, const uint8_t * pData,
                  uint32_t size, uint8_t mode)
{
  //  drvi2c_feusb_t *pDrvI2c = NULL;
	int result = 0;
	unsigned char data[255];
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_WriteGw");

    SONY_ARG_UNUSED(mode);

//    if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

//    pDrvI2c = (drvi2c_feusb_t*)(pI2c->user);
//    if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
#if 0
    data[0]=pI2c->gwSub;
    data[1]=deviceAddress;
    memcpy(&data[2],pData,size);

    result = write_rtl2832_stdi2c(pI2c->gwAddress,data,size+2,1);
	if(result)
	printf("drvi2c_feusb_WriteGw Err\n");
	//else
	//printf("drvi2c_feusb_WriteGw Success\n");

#else


{
	unsigned char data[128];
	data[0]=pI2c->gwSub;
   	 data[1]=deviceAddress;
    	memcpy(&data[2],pData,size);
	/*
		The Write gateway function pointer should:
		\verbatim {S} [Addr] [GW Sub] [TunerAddr] [Data0] [Data1] ... [DataN] {P} \endverbatim


	*/

//result = write_rtl2832_stdi2c(deviceAddress,&data[2],size);
result = write_rtl2832_stdi2c(deviceAddress,pData,size);
   
}

#endif
    switch (result) {
	    case 0:
	        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
			break;
	    case 1:
	        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
			break;
	    default:
		 SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
			break;
    }
/*
    switch(i2c_feusb_WriteGw(pDrvI2c->handle, deviceAddress, pData, size,
                              pI2c->gwAddress, pI2c->gwSub))
    {
    case I2C_FEUSB_OK:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    case I2C_FEUSB_ERROR_ACK:
    case I2C_FEUSB_ERROR_ACCESS:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
    default:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
    }
    */
}

sony_result_t drvi2c_feusb_ReadRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress,
                  uint8_t* pData, uint32_t size)
{
    //drvi2c_feusb_t *pDrvI2c = NULL;
	int result = 0;
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_ReadRegister");

//    if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

//    pDrvI2c = (drvi2c_feusb_t*)(pI2c->user);
//    if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
	result = I2cRead8(deviceAddress, subAddress, pData, size);
 if(result)
 printf("drvi2c_feusb_ReadRegister Err\n");
 //else
 //printf("drvi2c_feusb_ReadRegister Success\n");

	switch (result) {
	case 0:
		SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
		break;
	case 1:
		SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
		break;
	default:
	 SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
		break;
	}
/*
    switch(result)
    {
    case I2C_FEUSB_OK:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    case I2C_FEUSB_ERROR_ACK:
    case I2C_FEUSB_ERROR_ACCESS:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
    default:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
    }
*/	
}
#if 0
/* Driver initialization */
sony_result_t drvi2c_feusb_Initialize(drvi2c_feusb_t* pDrvI2c)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_I2C_ENTER("drvi2c_feusb_Initialize");

    if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    result = drvi2c_feusb_Initialize_InstanceId(pDrvI2c, 0);

    SONY_TRACE_I2C_RETURN(result);
}

sony_result_t drvi2c_feusb_Initialize_InstanceId(drvi2c_feusb_t* pDrvI2c, uint8_t instanceId)
{
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_Initialize_InstanceId");

    if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    pDrvI2c->instanceId = instanceId;

    switch(i2c_feusb_Initialize(&(pDrvI2c->handle), instanceId))
    {
    case I2C_FEUSB_OK:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    case I2C_FEUSB_ERROR_ACK:
    case I2C_FEUSB_ERROR_ACCESS:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
    default:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
    }
}

/* Driver finalization */
sony_result_t drvi2c_feusb_Finalize(drvi2c_feusb_t* pDrvI2c)
{
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_Finalize");

    if(!pDrvI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

    switch(i2c_feusb_Finalize(pDrvI2c->handle))
    {
    case I2C_FEUSB_OK:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    case I2C_FEUSB_ERROR_ACK:
    case I2C_FEUSB_ERROR_ACCESS:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_I2C);
    default:
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_OTHER);
    }
}
#endif
/* I2c object creation */
sony_result_t drvi2c_feusb_CreateI2c(sony_i2c_t* pI2c, drvi2c_feusb_t* pDrvI2c)
{
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_CreateI2c");

    if((!pI2c) || (!pDrvI2c)){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pI2c->Read = drvi2c_feusb_Read;
    pI2c->Write = drvi2c_feusb_Write;
    pI2c->ReadRegister = drvi2c_feusb_ReadRegister;
    pI2c->WriteRegister = sony_i2c_CommonWriteRegister;
    pI2c->WriteOneRegister = sony_i2c_CommonWriteOneRegister;
    pI2c->gwAddress = 0;
    pI2c->gwSub = 0;
    pI2c->user = pDrvI2c; /* Store driver object to user pointer */

    SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
}

/* I2c object creation for gateway access */
sony_result_t drvi2c_feusb_CreateI2cGw(sony_i2c_t* pI2c, drvi2c_feusb_t* pDrvI2c, uint8_t gwAddress, uint8_t gwSub)
{
    SONY_TRACE_I2C_ENTER("drvi2c_feusb_CreateI2cGw");

    if((!pI2c) || (!pDrvI2c)){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }

    pI2c->Read = drvi2c_feusb_ReadGw;
    pI2c->Write = drvi2c_feusb_WriteGw;
    pI2c->ReadRegister = sony_i2c_CommonReadRegister;
    pI2c->WriteRegister = sony_i2c_CommonWriteRegister;
    pI2c->WriteOneRegister = sony_i2c_CommonWriteOneRegister;
    pI2c->gwAddress = gwAddress;
    pI2c->gwSub = gwSub;
    pI2c->user = pDrvI2c; /* Store driver object to user pointer */

    SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
}


