/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-03-28 01:40:32 #$
  File Revision : $Revision:: 4945 $
------------------------------------------------------------------------------*/

#include "sony_i2c.h"

#include "sony_stdlib.h" /* for memcpy */

#define BURST_WRITE_MAX 128 /* Max length of burst write */
#if 1
sony_result_t sony2837_i2c_CommonReadRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t* pData, uint32_t size)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_I2C_ENTER("sony_i2c_CommonReadRegister");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }

    result = pI2c->Write(pI2c, deviceAddress, &subAddress, 1, SONY_I2C_START_EN);
    if(result == SONY_RESULT_OK){
        result = pI2c->Read(pI2c, deviceAddress, pData, size, SONY_I2C_START_EN | SONY_I2C_STOP_EN);
    }

    SONY_TRACE_I2C_RETURN(result);
}

sony_result_t sony2837_i2c_CommonWriteRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, const uint8_t* pData, uint32_t size)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t buffer[BURST_WRITE_MAX + 1];

    SONY_TRACE_I2C_ENTER("sony_i2c_CommonWriteRegister");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    if(size > BURST_WRITE_MAX){
        /* Buffer is too small... */
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    
    buffer[0] = subAddress;
    sony_memcpy(&(buffer[1]), pData, size);

    /* send the new buffer */
    result = pI2c->Write(pI2c, deviceAddress, buffer, size+1, SONY_I2C_START_EN | SONY_I2C_STOP_EN);
    SONY_TRACE_I2C_RETURN(result);
}

sony_result_t sony2837_i2c_CommonWriteOneRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_I2C_ENTER("sony_i2c_CommonWriteOneRegister");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    result = pI2c->WriteRegister(pI2c, deviceAddress, subAddress, &data, 1);
    SONY_TRACE_I2C_RETURN(result);
}

/* For Read-Modify-Write */
sony_result_t sony2837_i2c_SetRegisterBits(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress, uint8_t data, uint8_t mask)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_I2C_ENTER("sony_i2c_SetRegisterBits");

    if(!pI2c){
        SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
    }
    if(mask == 0x00){
        /* Nothing to do */
        SONY_TRACE_I2C_RETURN(SONY_RESULT_OK);
    }
    
    if(mask != 0xFF){
        uint8_t rdata = 0x00;
        result = pI2c->ReadRegister(pI2c, deviceAddress, subAddress, &rdata, 1);
        if(result != SONY_RESULT_OK){ SONY_TRACE_I2C_RETURN(result); }
        data = (uint8_t)((data & mask) | (rdata & (mask ^ 0xFF)));
    }

    result = pI2c->WriteOneRegister(pI2c, deviceAddress, subAddress, data);
    SONY_TRACE_I2C_RETURN(result);
}

#endif
extern int CXD2837_DemodType;
extern int cxd2837_bInit;
extern unsigned char TunerHV,TunMode;
extern unsigned char cxd2837adr ;

sony_result_t Cxd2837_I2c_Read(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t* pData,
                  uint32_t size, uint8_t mode)
{

	int result=0;
    SONY_TRACE_I2C_ENTER("Cxd2837_I2c_Read");

    SONY_ARG_UNUSED(mode);

    if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

	if((TunMode==31)||(TunMode==34)||(TunMode==44)||(TunMode==45)||(TunMode==42)||(TunMode==27))
	{//printf("CXD2837_DemodType=%d\n",CXD2837_DemodType);
		if(cxd2837adr!=0xda)
		{
			if(cxd2837_bInit==1){
				deviceAddress&=0xef;
			}
			else if(cxd2837_bInit==2){
				deviceAddress|=0x10;
			}
			else{
				if(CXD2837_DemodType==2){
					if(TunerHV==0)
						deviceAddress&=0xef;
					else
						deviceAddress|=0x10;
				}
				else if(CXD2837_DemodType==1){
					deviceAddress|=0x10;
				}
				else{
					deviceAddress&=0xef;
				}
			}
		}
	}

	result = read_rtl2832_stdi2c(deviceAddress, pData, size);
//if(result)
	//printf("2837 read err.\n");

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

sony_result_t Cxd2837_I2c_Write(sony_i2c_t* pI2c, uint8_t deviceAddress, const uint8_t * pData,
                  uint32_t size, uint8_t mode)
{
 
	int result = 0;
    SONY_TRACE_I2C_ENTER("Cxd2837_I2c_Write");

    SONY_ARG_UNUSED(mode);

    if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

		if((TunMode==31)||(TunMode==34)||(TunMode==44)||(TunMode==45)||(TunMode==42)||(TunMode==27))
		{
			if(cxd2837adr!=0xda)
			{
				if(cxd2837_bInit==1){
					deviceAddress&=0xef;
				}
				else if(cxd2837_bInit==2){
					deviceAddress|=0x10;
				}
				else{
					if(CXD2837_DemodType==2){
						if(TunerHV==0)
							deviceAddress&=0xef;
						else
							deviceAddress|=0x10;
					}
					else if(CXD2837_DemodType==1){
						deviceAddress|=0x10;
					}
					else{
						deviceAddress&=0xef;
					}
				}
			}
		}


    result = write_rtl2832_stdi2c(deviceAddress, pData, size,1);
//if(result)
//	printf("Cxd2837_I2c_Write err.\n");

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
sony_result_t Cxd2837_I2c_ReadGw(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t* pData,
                  uint32_t size, uint8_t mode)
{
    
	int result;
	unsigned char data[2];
	unsigned int sub_addr;
    SONY_TRACE_I2C_ENTER("Cxd2837_I2c_ReadGw");

    SONY_ARG_UNUSED(mode);

    if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

	sub_addr = pI2c->gwSub;
	sub_addr<<=8;
	sub_addr+=deviceAddress+1;
	result = I2cRead16_NoStop(pI2c->gwAddress, sub_addr,pData, size);

//if(result)
	//printf("Cxd2837_I2c_ReadGw err.\n");

	//	printf("Cxd2837_I2c_WriteGw deviceAddress:%x pI2c->gwAddress:%x pI2c->gwSub:%x\n",deviceAddress,pI2c->gwAddress,pI2c->gwSub);
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

/* For gateway write access */
sony_result_t Cxd2837_I2c_WriteGw(sony_i2c_t* pI2c, uint8_t deviceAddress, const uint8_t * pData,
                  uint32_t size, uint8_t mode)
{
 
	int result = 0;
	unsigned char data[255];
    SONY_TRACE_I2C_ENTER("Cxd2837_I2c_WriteGw");

    SONY_ARG_UNUSED(mode);

    if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);
	
    data[0]=pI2c->gwSub;
    data[1]=deviceAddress;
    memcpy(&data[2],pData,size);
	/*
		The Write gateway function pointer should:
		\verbatim {S} [Addr] [GW Sub] [TunerAddr] [Data0] [Data1] ... [DataN] {P} \endverbatim

	*/
//printf("%x %x %x %x %x %x \n",data[0],data[1],data[2],data[3],data[4],data[5]);
    result = write_rtl2832_stdi2c(pI2c->gwAddress,data,size+2,1);
if(result)

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
    switch(i2c_WriteGw(pDrvI2c->handle, deviceAddress, pData, size,
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
/*
sony_result_t Cxd2837_I2c_ReadRegister(sony_i2c_t* pI2c, uint8_t deviceAddress, uint8_t subAddress,
                  uint8_t* pData, uint32_t size)
{

	int result = 0;
    SONY_TRACE_I2C_ENTER("Cxd2837_I2c_ReadRegister");

    if(!pI2c) SONY_TRACE_I2C_RETURN(SONY_RESULT_ERROR_ARG);

	result = I2cRead8(deviceAddress, subAddress, pData, size);
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

*/

