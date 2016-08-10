/*------------------------------------------------------------------------------
  Copyright 2013 Sony Corporation

  Last Updated  : 2013/12/19
  File Revision : 1.1.0.0
------------------------------------------------------------------------------*/

#include "sony_common.h"
#include "sony_i2c.h"
#include "sony_ascot3.h"
#include "drvi2c_feusb.h"

//#include "sony_i2c_log.h"

#include <stdio.h>
//#include <stdlib.h>
#include <time.h>


unsigned char cxd2872adr = 0xc0;
sony_i2c_t i2c;
sony_i2c_t tunerI2C;

static sony_ascot3_t   g_tuner;

static int g_traceEnable = 0;
extern unsigned char DemMode,TunerSMode;

static void PrintResult(sony_result_t result);

int sony_cxd2872_init(void)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_result_t tuneResult = SONY_RESULT_OK;
    

printf("[cxd2872_init]-->1\n");
    uint8_t i = 0;

    /*------------------------------------------------------------------------------
     Setup / Initialisation
    ------------------------------------------------------------------------------*/
    /* Create I2C interface for tuner and demodulator parts.  GW (GateWay) members 
       provided but not required for tuner communication.  I2C switching handled 
       internally in the driver using a repeater. */
      
    i2c.gwAddress = 0xc8;                                   /* N/A */
    i2c.gwSub = 0x09;                                       /* N/A */
    i2c.Read = drvi2c_feusb_Read;                           /* Base level HW interfacing I2C read function */
    i2c.Write = drvi2c_feusb_Write;                         /* Base level HW interfacing I2C write function */
    i2c.ReadRegister = sony_i2c_CommonReadRegister;         /* Common wrapper function for multi byte Read operation */
    i2c.WriteRegister = sony_i2c_CommonWriteRegister;       /* Common wrapper function for multi byte Write operation */
    i2c.WriteOneRegister = sony_i2c_CommonWriteOneRegister; /* Common wrapper function for single byte Write operation */
#if 1
	    /* Setup I2C interfaces. */
    tunerI2C.gwAddress = 0xc8;
    tunerI2C.gwSub = 0x09;      /* Connected via demod I2C gateway function. */
    tunerI2C.Read = drvi2c_feusb_ReadGw;
    tunerI2C.Write = drvi2c_feusb_WriteGw;
    tunerI2C.ReadRegister = sony_i2c_CommonReadRegister;
    tunerI2C.WriteRegister = sony_i2c_CommonWriteRegister;
    tunerI2C.WriteOneRegister = sony_i2c_CommonWriteOneRegister;
#endif
	uint8_t i2cAddress = 0;
	sony_ascot3_xtal_t xtalFreq = 0;
	uint32_t flags = 0;
printf("[cxd2872_init]-->2\n");
		xtalFreq = SONY_ASCOT3_XTAL_16000KHz;
	//	flags |= SONY_ASCOT3_CONFIG_LOOPTHRU_ENABLE;   //LOOPTHRU enable
	//	flags |= SONY_ASCOT3_CONFIG_REFOUT_400mVpp;	   //REFOUT enable, output level is 400mVp-p.
		flags |= SONY_ASCOT3_CONFIG_IFAGCSEL_ALL2;     //IF/AGC 2  is used for both Analog and Digital,  cxd2872 only have IF/AGC2
	//	flags |= SONY_ASCOT3_CONFIG_LOOPFILTER_INTERNAL;
	//	flags |= SONY_ASCOT3_CONFIG_OVERLOAD_STANDARD;
	//	flags |= SONY_ASCOT3_CONFIG_RFIN_MATCHING_ENABLE; // if LOOPTHRU_DISABLE use it
	//	flags |= SONY_ASCOT3_CONFIG_EXT_REF; //if xtalFreq=41MHz
		i2cAddress = cxd2872adr;
printf("[cxd2872_init]-->3\n");
	if(DemMode==35){ //cxd2840
printf("[cxd2872_init]-->3.0\n");
		result = sony_ascot3_Create(&g_tuner, xtalFreq, i2cAddress, &tunerI2C, flags);
	}
	else if(DemMode==37){//cxd2837
		result = sony_ascot3_Create(&g_tuner, xtalFreq, i2cAddress, &i2c, flags);
	}
	else if(DemMode==38){//mn88436

		result = sony_ascot3_Create(&g_tuner, xtalFreq, i2cAddress, &i2c, flags);
	}
	else
		result = sony_ascot3_Create(&g_tuner, xtalFreq, i2cAddress, &i2c, flags);
printf("[cxd2872_init]-->3.1\n");	
	if(result != SONY_RESULT_OK){
		printf("Tuner ascot3_Create failure!\n");
		}
	//PrintResult(result);
	
	/* First initialization */
	result = sony_ascot3_Initialize(&g_tuner);
printf("[cxd2872_init]-->3.2\n");
	if(result != SONY_RESULT_OK){
		printf("Tuner initialization failure!\n");
		}
	//PrintResult(result);
	/* RSSI_IFOUT_TGT 2837--DVBT,T2,C2:E0,DVBC:D4 */
	uint8_t cdata = 0xd4;
	uint8_t tdata = 0xe0;
#if 1
	if(DemMode==37){ //cxd2837
		if(TunerSMode==0)//DVB-C
			result = g_tuner.pI2c->WriteRegister(g_tuner.pI2c, g_tuner.i2cAddress, 0x79, &cdata, 1);
		else //DVBT,T2,
			result = g_tuner.pI2c->WriteRegister(g_tuner.pI2c, g_tuner.i2cAddress, 0x79, &tdata, 1);
		}
	if(DemMode==35){ //cxd2840
printf("DTMB Single:0xb4 DTMB Multi:0xbc.\n");
		 cdata = 0xdC;
		 tdata = 0xB4;//DTMB Single:0xb4 DTMB Multi:0xbc	
		if(TunerSMode==0)//DVB-C
			result = g_tuner.pI2c->WriteRegister(g_tuner.pI2c, g_tuner.i2cAddress, 0x79, &cdata, 1);
		else //DTMB
			result = g_tuner.pI2c->WriteRegister(g_tuner.pI2c, g_tuner.i2cAddress, 0x79, &tdata, 1);
		}
	
	if(result != SONY_RESULT_OK){
		printf("TunerWrite reg 0x79 err!\n");
		}
#endif
	printf("[cxd2872_init]-->4\n");
	return result;
}

void sony_cxd2872__SetFreq(unsigned char mode,unsigned long FrequencyKHz,unsigned long bandwidth)
{
	sony_result_t result = SONY_RESULT_OK;
	sony_ascot3_tv_system_t system = SONY_ASCOT3_TV_SYSTEM_UNKNOWN;
	printf("[sony_cxd2872__SetFreq]-->1  %d  %d  %d\n",mode,FrequencyKHz,bandwidth);
	//	sony_cxd2872_init();
	printf("[sony_cxd2872__SetFreq]-->2\n");
	if(DemMode==37)
	{//cxd2837
		if(mode==0){			
			if(bandwidth>6090){
				system = SONY_ASCOT3_DTV_DVBC_8;
			}
	//		else if(bandwidth<5362){
	//			system = SONY_ASCOT3_DTV_DVBC_6;
	//		}
			else{
				system = SONY_ASCOT3_DTV_DVBC_6;
			}

		}
		else if(mode==1){
			if(bandwidth==7000)
				system = SONY_ASCOT3_DTV_DVBT_7;
			else if(bandwidth==8000)
				system = SONY_ASCOT3_DTV_DVBT_8;
			else
				system = SONY_ASCOT3_DTV_DVBT_6;;
		}
	
		else{
			if(bandwidth==7000)
				system = SONY_ASCOT3_DTV_DVBT2_7;
			else if(bandwidth==8000)
				system = SONY_ASCOT3_DTV_DVBT2_8;
			else
				system = SONY_ASCOT3_DTV_DVBT2_6;
		}
	}	
	else if(DemMode==35){//cxd2840
		if(mode==0){
			if(bandwidth>6090)
				system = SONY_ASCOT3_DTV_DVBC_8;
			else
				system = SONY_ASCOT3_DTV_DVBC_6;	
		}
	
		else{
				system = SONY_ASCOT3_DTV_DTMB;
		}
	}	
	else if(DemMode==38){	//MN88436 	
		if(mode==0){
			system = SONY_ASCOT3_DTV_8VSB;
		}
				
		else{
			system = SONY_ASCOT3_DTV_QAM;
		}
	}
	else if(DemMode==13){	//TDA10024
			system = SONY_ASCOT3_DTV_DVBC_8;
	}

	else{
		if(mode==0){			
			if(bandwidth>6090){
				system = SONY_ASCOT3_DTV_DVBC_8;
			}
			else{
				system = SONY_ASCOT3_DTV_DVBC_6;
			}
	
		}
		else if(mode==1){
			if(bandwidth==7000)
				system = SONY_ASCOT3_DTV_DVBT_7;
			else if(bandwidth==8000)
				system = SONY_ASCOT3_DTV_DVBT_8;
			else
				system = SONY_ASCOT3_DTV_DVBT_6;;
		}
		
		else{
			if(bandwidth==7000)
				system = SONY_ASCOT3_DTV_DVBT2_7;
			else if(bandwidth==8000)
				system = SONY_ASCOT3_DTV_DVBT2_8;
			else
				system = SONY_ASCOT3_DTV_DVBT2_6;
		}
	}	

	result = sony_ascot3_Tune(&g_tuner, FrequencyKHz, system);
	if(result != SONY_RESULT_OK)
		printf("sony_ascot3_Tune err\n");
	PrintResult(result);	
//	SONY_SLEEP(5);
	result = sony_ascot3_TuneEnd(&g_tuner);
	if(result != SONY_RESULT_OK)
		printf("sony_ascot3_TuneEnd err\n");
   	PrintResult(result);
			

}
#if 0
unsigned char cxd2872_GetRFlevel(void){

	int32_t rxPwrPtr;

	sony_ascot3_ReadRssi(&g_tuner, &rxPwrPtr);
	
	if(rxPwrPtr>=2048){
		rxPwrPtr-=2048;
		rxPwrPtr/=2047;
		rxPwrPtr/=2;
		rxPwrPtr=100-rxPwrPtr;
		}
	else{
		rxPwrPtr/=2047;
		rxPwrPtr/=2;
		}
	return rxPwrPtr;

}
#endif
int cxd2872_GetRFlevel(void)
{

	int rxPwrPtr;
	
	sony_result_t result = SONY_RESULT_OK;

	sony_ascot3_ReadRssi(&g_tuner, &rxPwrPtr);
	PrintResult(result);
	printf("RSSI: %.02f\n", rxPwrPtr/100.0);
	rxPwrPtr/=100;
#if 0
	rxPwrPtr+=100;
    	if(rxPwrPtr<0)
		rxPwrPtr=0;
	if(rxPwrPtr>99)
		rxPwrPtr=99;
#endif
	return (int)rxPwrPtr;
}



static void PrintResult(sony_result_t result)
{
	switch(result)
	{
	case SONY_RESULT_OK:
	printf("Success.\n");
	break;
	case SONY_RESULT_ERROR_ARG:
	printf("ERROR: Invalid argument. (maybe software bug)\n");
	break;
	case SONY_RESULT_ERROR_I2C:
	printf("ERROR: I2C communication error.\n");
	break;
	case SONY_RESULT_ERROR_SW_STATE:
	printf("ERROR: Invalid software state.\n");
	break;
	case SONY_RESULT_ERROR_HW_STATE:
	printf("ERROR: Invalid hardware state.\n");
	break;
	case SONY_RESULT_ERROR_TIMEOUT:
	printf("ERROR: Timeout occured.\n");
	break;
	case SONY_RESULT_ERROR_UNLOCK:
	printf("ERROR: Failed to lock.\n");
	break;
	case SONY_RESULT_ERROR_RANGE:
	printf("ERROR: Out of range.\n");
	break;
	case SONY_RESULT_ERROR_NOSUPPORT:
	printf("ERROR: This parameter is not supported.\n");
	break;
	case SONY_RESULT_ERROR_CANCEL:
	printf("ERROR: The operation is canceled.\n");
	break;
	case SONY_RESULT_ERROR_OTHER:
	printf("ERROR: Other Error.\n");
	break;
	default:
	printf("Unknown result code.\n");
	break;
	}
}

#if 0
/* Utility functions */
static void GetALine(char* pStr, int nSize);
static void PrintDumpData(unsigned char *pData, int nSize);
static void PrintResult(sony_result_t result);

/*------------------------------------------------------------------------------
  Menu functions (static)
------------------------------------------------------------------------------*/

static sony_result_t Monitor_CurrentState(void);

static sony_result_t TunerTune(void);
static sony_result_t ShiftFRF(void);
static sony_result_t TunerSleep(void);

static sony_result_t SetTunerGPO(void);
static sony_result_t RFFilterConfig(void);
static sony_result_t ReadRssi(void);

static sony_result_t TunerDump(void);
static sony_result_t ToggleTrace(void);
static sony_result_t ToggleI2cLog(void);
static sony_result_t Exit(void);

static void FinalizeI2cLog(void);

/*------------------------------------------------------------------------------
  Struct for menu
------------------------------------------------------------------------------*/
typedef sony_result_t (*app_function_t)(void);
#define COND_SLEEP      0x01
#define COND_ACTIVE     0x02
#define COND_ALL        (COND_SLEEP | COND_ACTIVE)

typedef struct menu_function_t
{
    app_function_t function;
    const char* pName;
    uint32_t condition;
} menu_function_t;

static menu_function_t g_Functions[] = {
    {Monitor_CurrentState, "Current State",              COND_ALL},

    {TunerTune,            "Tuner Tune",                 COND_ALL},
    {ShiftFRF,             "Shift FRF",                  COND_ACTIVE},
    {TunerSleep,           "Tuner Sleep",                COND_ALL},

    {SetTunerGPO,          "Set Tuner GPO",              COND_ALL},
    {RFFilterConfig,       "RF Filter Config",           COND_ALL},
    {ReadRssi,             "Read RSSI",                  COND_ACTIVE},

    {TunerDump,            "Tuner Register Dump",        COND_ALL},
    {ToggleTrace,          "Toggle Trace",               COND_ALL},
    {ToggleI2cLog,         "Toggle I2cLog",              COND_ALL},
    {Exit,                 "Exit",                       COND_ALL}
};

/*------------------------------------------------------------------------------
  I2C driver <PORTING>
------------------------------------------------------------------------------*/
/* Modify if choose another I2C driver */
#include "drvi2c_feusb.h"
static drvi2c_feusb_t   g_feusb; /* FEUSB dependent */
#define I2C_FINALIZE()  drvi2c_feusb_Finalize(&g_feusb)

/*------------------------------------------------------------------------------
  CUI application main
------------------------------------------------------------------------------*/
static sony_i2c_t       g_i2cTuner;
static sony_ascot3_t   g_tuner;

/* Structs for i2c log using sony_i2c_log.c/h */
static sony_i2c_t       g_i2cTunerLog;
static sony_i2c_log_t   g_logTuner;

static int g_traceEnable = 0;
static FILE* g_i2cLogFile = NULL;

int main(int argc, char** argv)
{
    char strInput[16]; /* User input buffer */
    sony_result_t result = SONY_RESULT_OK;
    int isConfigAscot3r = 0;
    menu_function_t* enableEntry[sizeof(g_Functions)/sizeof(g_Functions[0])];

    printf("========================================================\n");
    printf("        Sony silicon tuner ASCOT3 reference code        \n");
    printf("               Sample application CUI                   \n");
    printf("                  Version: %s\n", SONY_ASCOT3_VERSION);
    printf("========================================================\n\n");

    /* Choose trace enable/disable */
    printf("Select Trace Enable(1)/Disable(0)\n");
    GetALine(strInput, sizeof(strInput));
    switch(strInput[0]){
    case '0':
    default:
        g_traceEnable = 0;
        break;
    case '1':
        g_traceEnable = 1;
        break;
    }

    /* I2C initialization <PORTING> --------------------------- */
    if(drvi2c_feusb_Initialize(&g_feusb) != SONY_RESULT_OK){
        printf("I2c initialization failure!\n");
        getchar();
        return -1;
    }

    printf("Sony Silicon Tuner Access is via I2C Gateway(1) / normal I2C(0)?\n");
    GetALine(strInput, sizeof(strInput));
    if(atoi(strInput)){
        uint32_t addressGw = 0;
        /* I2C Gateway */
        printf("Input I2C Gateway Slave Address in 8bit form. (Hex)\n");
        GetALine(strInput, sizeof(strInput));
        addressGw = strtoul(strInput, NULL, 16);
        drvi2c_feusb_CreateI2cGw(&g_i2cTuner, &g_feusb, (uint8_t)addressGw, 0x09);
    }else{
        drvi2c_feusb_CreateI2c(&g_i2cTuner, &g_feusb);
    }
    /* -------------------------------------------------------- */

    /* I2c structs (for logging) initialization */
    sony_i2c_CreateI2cLog(&g_i2cTunerLog, &g_i2cTuner, &g_logTuner);

    /* Product selection */
    printf("Product Type Selection.\n");
    printf("0: CXD2871/2872\n");
    printf("1: CXD2875\n");
    GetALine(strInput, sizeof(strInput));
    if(atoi(strInput)){
        isConfigAscot3r = 1;
    }

    /* Tuner instance creation */
    if(isConfigAscot3r){
        /* CXD2875 */
        uint8_t i2cAddress = 0;
        uint32_t flags = 0;

        printf("Input I2C Slave Address of ASCOT in 8bit form. (Hex) (Default: C0)\n");
        GetALine(strInput, sizeof(strInput));
        i2cAddress = (uint8_t)strtoul(strInput, NULL, 16);
        if(i2cAddress == 0){
            i2cAddress = SONY_ASCOT3_ADDRESS;
        }

        if(!(flags & SONY_ASCOT3_CONFIG_EXT_REF)){
            printf("Disable Xtal in Sleep State ? (0: No, 1: Yes)\n");
            GetALine(strInput, sizeof(strInput));
            if(atoi(strInput)){
                flags |= SONY_ASCOT3_CONFIG_SLEEP_DISABLEXTAL;
            }
        }

        printf("REFOUT setting.\n");
        printf("0: Disable\n");
        printf("1: 500mVp-p\n");
        printf("2: 400mVp-p\n");
        printf("3: 600mVp-p\n");
        printf("4: 800mVp-p\n");
        GetALine(strInput, sizeof(strInput));
        switch(atoi(strInput)){
        case 0:
            break;
        case 1:
            flags |= SONY_ASCOT3_CONFIG_REFOUT_500mVpp; break;
        case 2:
            flags |= SONY_ASCOT3_CONFIG_REFOUT_400mVpp; break;
        case 3:
            flags |= SONY_ASCOT3_CONFIG_REFOUT_600mVpp; break;
        case 4:
            flags |= SONY_ASCOT3_CONFIG_REFOUT_800mVpp; break;
        default:
            printf("Invalid input. Setting 0 is selected.\n");
            break;
        }

        sony_ascot3_2875_Create(&g_tuner, i2cAddress, &g_i2cTunerLog, flags);
    }else{
        /* CXD2871/2872 */
        uint8_t i2cAddress = 0;
        sony_ascot3_xtal_t xtalFreq = 0;
        uint32_t flags = 0;

        printf("Input I2C Slave Address of ASCOT in 8bit form. (Hex) (Default: C0)\n");
        GetALine(strInput, sizeof(strInput));
        i2cAddress = (uint8_t)strtoul(strInput, NULL, 16);
        if(i2cAddress == 0){
            i2cAddress = SONY_ASCOT3_ADDRESS;
        }

        printf("Xtal Frequency Setting.\n");
        printf("0: 16 MHz\n");
        printf("1: 20.5 MHz\n");
        printf("2: 24 MHz\n");
        printf("3: 41 MHz (+ External Ref Input)\n");
        GetALine(strInput, sizeof(strInput));
        switch(atoi(strInput)){
        case 0:
            xtalFreq = SONY_ASCOT3_XTAL_16000KHz; break;
        case 1:
            xtalFreq = SONY_ASCOT3_XTAL_20500KHz; break;
        case 2:
            xtalFreq = SONY_ASCOT3_XTAL_24000KHz; break;
        case 3:
            xtalFreq = SONY_ASCOT3_XTAL_41000KHz;
            flags |= SONY_ASCOT3_CONFIG_EXT_REF;
            break;
        default:
            printf("Invalid input. Setting 0 is selected.\n");
            xtalFreq = SONY_ASCOT3_XTAL_16000KHz;
            break;
        }

        if(xtalFreq != SONY_ASCOT3_XTAL_41000KHz){
            printf("Use External Oscillator ? (0: No, 1: Yes)\n");
            GetALine(strInput, sizeof(strInput));
            if(atoi(strInput)){
                flags |= SONY_ASCOT3_CONFIG_EXT_REF;
            }
        }

        if(!(flags & SONY_ASCOT3_CONFIG_EXT_REF)){
            printf("Disable Xtal in Sleep State ? (0: No, 1: Yes)\n");
            GetALine(strInput, sizeof(strInput));
            if(atoi(strInput)){
                flags |= SONY_ASCOT3_CONFIG_SLEEP_DISABLEXTAL;
            }
        }

        printf("Internal RFAGC (Overload) Setting. (0: Extended TC, 1: Standard)\n");
        GetALine(strInput, sizeof(strInput));
        if(atoi(strInput)){
            flags |= SONY_ASCOT3_CONFIG_OVERLOAD_STANDARD;
        }

        printf("Loop Filter Setting. (0: External, 1: Internal)\n");
        GetALine(strInput, sizeof(strInput));
        if(atoi(strInput)){
            flags |= SONY_ASCOT3_CONFIG_LOOPFILTER_INTERNAL;
        }

        printf("Loop through Setting. (Only for CXD2872) (0: Disable, 1: Enable)\n");
        GetALine(strInput, sizeof(strInput));
        if(atoi(strInput)){
            flags |= SONY_ASCOT3_CONFIG_LOOPTHRU_ENABLE;
        }

        printf("RFIN Matching Setting. (0: Disable, 1: Enable)\n");
        GetALine(strInput, sizeof(strInput));
        if(atoi(strInput)){
            flags |= SONY_ASCOT3_CONFIG_RFIN_MATCHING_ENABLE;
        }

        printf("Tuner IFOUT/AGCIN Configuration.\n");
        printf("0: Analog & Digital: IFOUT1/AGCIN1\n");
        printf("1: Analog & Digital: IFOUT2/AGCIN2\n");
        printf("2: Analog: IFOUT1/AGCIN1, Digital: IFOUT2/AGCIN2\n");
        printf("3: Analog: IFOUT2/AGCIN2, Digital: IFOUT1/AGCIN1\n");
        GetALine(strInput, sizeof(strInput));
        switch(atoi(strInput)){
        case 0:
            flags |= SONY_ASCOT3_CONFIG_IFAGCSEL_ALL1; break;
        case 1:
            flags |= SONY_ASCOT3_CONFIG_IFAGCSEL_ALL2; break;
        case 2:
            flags |= SONY_ASCOT3_CONFIG_IFAGCSEL_A1D2; break;
        case 3:
            flags |= SONY_ASCOT3_CONFIG_IFAGCSEL_D1A2; break;
        default:
            printf("Invalid input. Setting 2 is selected.\n");
            flags |= SONY_ASCOT3_CONFIG_IFAGCSEL_A1D2;
            break;
        }

        printf("REFOUT setting.\n");
        printf("0: Disable\n");
        printf("1: 500mVp-p\n");
        printf("2: 400mVp-p\n");
        printf("3: 600mVp-p\n");
        printf("4: 800mVp-p\n");
        GetALine(strInput, sizeof(strInput));
        switch(atoi(strInput)){
        case 0:
            break;
        case 1:
            flags |= SONY_ASCOT3_CONFIG_REFOUT_500mVpp; break;
        case 2:
            flags |= SONY_ASCOT3_CONFIG_REFOUT_400mVpp; break;
        case 3:
            flags |= SONY_ASCOT3_CONFIG_REFOUT_600mVpp; break;
        case 4:
            flags |= SONY_ASCOT3_CONFIG_REFOUT_800mVpp; break;
        default:
            printf("Invalid input. Setting 0 is selected.\n");
            break;
        }

        sony_ascot3_Create(&g_tuner, xtalFreq, i2cAddress, &g_i2cTunerLog, flags);

        if(!(flags & SONY_ASCOT3_CONFIG_EXT_REF)){
            uint8_t value = 0;

            printf("Input XOSC_SEL value. (If 0, default value is used.)\n");
            GetALine(strInput, sizeof(strInput));
            value = (uint8_t)strtoul(strInput, NULL, 0);
            if(value != 0){
                g_tuner.xosc_sel = value;
            }

            printf("Input XOSC_CAP_SET value. (If 0, default value is used.)\n");
            GetALine(strInput, sizeof(strInput));
            value = (uint8_t)strtoul(strInput, NULL, 0);
            if(value != 0){
                g_tuner.xosc_cap_set = value;
            }
        }
    }

    /* Choose i2cLog enable/disable */
    printf("Select I2c Log Enable(1)/Disable(0)\n");
    GetALine(strInput, sizeof(strInput));
    switch(strInput[0]){
    case '0':
    default:
        break;
    case '1':
        ToggleI2cLog();
        break;
    }

    /* First initialization */
    result = sony_ascot3_Initialize(&g_tuner);
    if(result != SONY_RESULT_OK){
        printf("Tuner initialization failure!\n");
        PrintResult(result);
        I2C_FINALIZE();
        FinalizeI2cLog();
        getchar();
        return -1;
    }

#ifdef SONY_ASCOT3_IGNORE_NVM_ERROR
    {
        /* May cause NVM read error but ignored in sony_ascot3_Initialize() */
        /* Print CPU_ERR/NVM_STT registers here */
        uint8_t data = 0;
        g_tuner.pI2c->ReadRegister(g_tuner.pI2c, g_tuner.i2cAddress, 0x1B, &data, 1);
        printf("CPU_ERR: 0x%02x\n", data);
        g_tuner.pI2c->ReadRegister(g_tuner.pI2c, g_tuner.i2cAddress, 0x1C, &data, 1);
        printf("NVM_STT: 0x%02x\n", data);
        if(data & 0x20){
            printf("Warning: Ecc Error was detected in Page0\n");
        }
        if(data & 0x02){
            printf("Warning: Ecc Error was detected in Page1\n");
        }
        if(data & 0x80){
            printf("Warning: Default NVM Data is used for Page0\n");
        }
        if(data & 0x08){
            printf("Warning: Default NVM Data is used for Page1\n");
        }
    }
#endif /* SONY_ASCOT3_IGNORE_NVM_ERROR */

    printf("System is Initialized Successfully!\n");

    /* Main loop */
    while(1){
        uint32_t cond = 0;
        int enableEntryNum = 0;
        int i = 0;
        int num = 0;

        /* Check current status and make enableEntry[] */
        if(g_tuner.state == SONY_ASCOT3_STATE_SLEEP){
            cond = COND_SLEEP;
        }else if(g_tuner.state == SONY_ASCOT3_STATE_ACTIVE){
            cond = COND_ACTIVE;
        }else{
            printf("Invalid SW state!\n");
            break;
        }

        /* Add table entry pointer to enableEntry[] */
        printf("\nInput Command Number\n");
        for(i=0; i<sizeof(g_Functions)/sizeof(g_Functions[0]); i++){
            if(g_Functions[i].condition & cond){
                enableEntry[enableEntryNum] = &g_Functions[i];
                enableEntryNum++;
            }
        }
        /* Print menu */
        for(i=0; i<enableEntryNum; i++){
            printf("%2d: %-30s", i, enableEntry[i]->pName);
            if(i % 2 == 1){
                printf("\n");
            }
        }
        if(enableEntryNum % 2 == 1){
            printf("\n");
        }

        GetALine(strInput, sizeof(strInput));

        num = atoi(strInput);
        if((num < 0) || (num >= enableEntryNum)){
            printf("Invalid Command Number!\n");
            continue;
        }

        /* Do function in g_Functions table */
        result = enableEntry[num]->function();
        if((result == SONY_RESULT_ERROR_ARG) || (result == SONY_RESULT_ERROR_I2C)
        || (result == SONY_RESULT_ERROR_SW_STATE)){
            break; /* Fatal error -> break; */
        }
    }

    I2C_FINALIZE();
    FinalizeI2cLog();
    getchar();
    return -1;
}

/*------------------------------------------------------------------------------
  Implementation of Static functions (Common)
------------------------------------------------------------------------------*/
static sony_result_t Monitor_CurrentState(void)
{
    printf("Frequency: %f MHz\n", (double)g_tuner.frequencykHz / 1000.0);

    printf("TV system: ");
    switch(g_tuner.tvSystem){
    case SONY_ASCOT3_TV_SYSTEM_UNKNOWN:
        printf("Unknown"); break;
    /* Analog */
    case SONY_ASCOT3_ATV_MN_EIAJ:  /* System-M (Japan) */
        printf("System-M (Japan)"); break;
    case SONY_ASCOT3_ATV_MN_SAP:   /* System-M (US) */
        printf("System-M (US)"); break;
    case SONY_ASCOT3_ATV_MN_A2:    /* System-M (Korea) */
        printf("System-M (Korea)"); break;
    case SONY_ASCOT3_ATV_BG:       /* System-B/G */
        printf("System-B/G"); break;
    case SONY_ASCOT3_ATV_I:        /* System-I */
        printf("System-I"); break;
    case SONY_ASCOT3_ATV_DK:       /* System-D/K */
        printf("System-D/K"); break;
    case SONY_ASCOT3_ATV_L:        /* System-L */
        printf("System-L"); break;
    case SONY_ASCOT3_ATV_L_DASH:   /* System-L DASH */
        printf("System-L DASH"); break;
    /* Digital */
    case SONY_ASCOT3_DTV_8VSB:     /* ATSC 8VSB */
        printf("ATSC 8VSB"); break;
    case SONY_ASCOT3_DTV_QAM:      /* US QAM */
        printf("US QAM"); break;
    case SONY_ASCOT3_DTV_ISDBT_6:  /* ISDB-T 6MHzBW */
        printf("ISDB-T 6MHzBW"); break;
    case SONY_ASCOT3_DTV_ISDBT_7:  /* ISDB-T 7MHzBW */
        printf("ISDB-T 7MHzBW"); break;
    case SONY_ASCOT3_DTV_ISDBT_8:  /* ISDB-T 8MHzBW */
        printf("ISDB-T 8MHzBW"); break;
    case SONY_ASCOT3_DTV_DVBT_5:   /* DVB-T 5MHzBW */
        printf("DVB-T 5MHzBW"); break;
    case SONY_ASCOT3_DTV_DVBT_6:   /* DVB-T 6MHzBW */
        printf("DVB-T 6MHzBW"); break;
    case SONY_ASCOT3_DTV_DVBT_7:   /* DVB-T 7MHzBW */
        printf("DVB-T 7MHzBW"); break;
    case SONY_ASCOT3_DTV_DVBT_8:   /* DVB-T 8MHzBW */
        printf("DVB-T 8MHzBW"); break;
    case SONY_ASCOT3_DTV_DVBT2_1_7:  /* DVB-T2 1.7MHzBW */
        printf("DVB-T2 1.7MHzBW"); break;
    case SONY_ASCOT3_DTV_DVBT2_5:  /* DVB-T2 5MHzBW */
        printf("DVB-T2 5MHzBW"); break;
    case SONY_ASCOT3_DTV_DVBT2_6:  /* DVB-T2 6MHzBW */
        printf("DVB-T2 6MHzBW"); break;
    case SONY_ASCOT3_DTV_DVBT2_7:  /* DVB-T2 7MHzBW */
        printf("DVB-T2 7MHzBW"); break;
    case SONY_ASCOT3_DTV_DVBT2_8:  /* DVB-T2 8MHzBW */
        printf("DVB-T2 8MHzBW"); break;
    case SONY_ASCOT3_DTV_DVBC_6:   /* DVB-C 6MHzBW */
        printf("DVB-C 6MHzBW"); break;
    case SONY_ASCOT3_DTV_DVBC_8:   /* DVB-C 8MHzBW */
        printf("DVB-C 8MHzBW"); break;
    case SONY_ASCOT3_DTV_DVBC2_6:  /* DVB-C2 6MHzBW */
        printf("DVB-C2 6MHzBW"); break;
    case SONY_ASCOT3_DTV_DVBC2_8:  /* DVB-C2 8MHzBW */
        printf("DVB-C2 8MHzBW"); break;
    case SONY_ASCOT3_DTV_DTMB:     /* DTMB */
        printf("DTMB"); break;
    default:
        printf("Invalid system value"); break;
    }
    printf("\n");

    return SONY_RESULT_OK;
}

static sony_result_t TunerTune(void)
{
    sony_result_t result = SONY_RESULT_OK;
    char strInput[16];

    uint32_t frequency = 0;
    sony_ascot3_tv_system_t system = SONY_ASCOT3_TV_SYSTEM_UNKNOWN;

    /* Frequency */
    printf("Input Frequency(MHz)\n");
    GetALine(strInput, sizeof(strInput));
    frequency = (uint32_t)(atof(strInput) * 1000.0);

    /* System */
    printf("Input TV System\n");
    /* Analog */
    printf("System-M (Japan) : %d\n", (int)SONY_ASCOT3_ATV_MN_EIAJ);  /* System-M (Japan) */
    printf("System-M (US)    : %d\n", (int)SONY_ASCOT3_ATV_MN_SAP);   /* System-M (US) */
    printf("System-M (Korea) : %d\n", (int)SONY_ASCOT3_ATV_MN_A2);    /* System-M (Korea) */
    printf("System-B/G       : %d\n", (int)SONY_ASCOT3_ATV_BG);       /* System-B/G */
    printf("System-I         : %d\n", (int)SONY_ASCOT3_ATV_I);        /* System-I */
    printf("System-D/K       : %d\n", (int)SONY_ASCOT3_ATV_DK);       /* System-D/K */
    printf("System-L         : %d\n", (int)SONY_ASCOT3_ATV_L);        /* System-L */
    printf("System-L DASH    : %d\n", (int)SONY_ASCOT3_ATV_L_DASH);   /* System-L DASH */
    /* Digital */
    printf("ATSC 8VSB        : %d\n", (int)SONY_ASCOT3_DTV_8VSB);     /* ATSC 8VSB */
    printf("US QAM           : %d\n", (int)SONY_ASCOT3_DTV_QAM);      /* US QAM */
    printf("ISDB-T 6MHzBW    : %d\n", (int)SONY_ASCOT3_DTV_ISDBT_6);  /* ISDB-T 6MHzBW */
    printf("ISDB-T 7MHzBW    : %d\n", (int)SONY_ASCOT3_DTV_ISDBT_7);  /* ISDB-T 7MHzBW */
    printf("ISDB-T 8MHzBW    : %d\n", (int)SONY_ASCOT3_DTV_ISDBT_8);  /* ISDB-T 8MHzBW */
    printf("DVB-T 5MHzBW     : %d\n", (int)SONY_ASCOT3_DTV_DVBT_5);   /* DVB-T 5MHzBW */
    printf("DVB-T 6MHzBW     : %d\n", (int)SONY_ASCOT3_DTV_DVBT_6);   /* DVB-T 6MHzBW */
    printf("DVB-T 7MHzBW     : %d\n", (int)SONY_ASCOT3_DTV_DVBT_7);   /* DVB-T 7MHzBW */
    printf("DVB-T 8MHzBW     : %d\n", (int)SONY_ASCOT3_DTV_DVBT_8);   /* DVB-T 8MHzBW */
    printf("DVB-T2 1.7MHzBW  : %d\n", (int)SONY_ASCOT3_DTV_DVBT2_1_7);/* DVB-T2 1.7MHzBW */
    printf("DVB-T2 5MHzBW    : %d\n", (int)SONY_ASCOT3_DTV_DVBT2_5);  /* DVB-T2 5MHzBW */
    printf("DVB-T2 6MHzBW    : %d\n", (int)SONY_ASCOT3_DTV_DVBT2_6);  /* DVB-T2 6MHzBW */
    printf("DVB-T2 7MHzBW    : %d\n", (int)SONY_ASCOT3_DTV_DVBT2_7);  /* DVB-T2 7MHzBW */
    printf("DVB-T2 8MHzBW    : %d\n", (int)SONY_ASCOT3_DTV_DVBT2_8);  /* DVB-T2 8MHzBW */
    printf("DVB-C 6MHzBW     : %d\n", (int)SONY_ASCOT3_DTV_DVBC_6);   /* DVB-C 6MHzBW */
    printf("DVB-C 8MHzBW     : %d\n", (int)SONY_ASCOT3_DTV_DVBC_8);   /* DVB-C 8MHzBW */
    printf("DVB-C2 6MHzBW    : %d\n", (int)SONY_ASCOT3_DTV_DVBC2_6);  /* DVB-C2 6MHzBW */
    printf("DVB-C2 8MHzBW    : %d\n", (int)SONY_ASCOT3_DTV_DVBC2_8);  /* DVB-C2 8MHzBW */
    printf("DTMB             : %d\n", (int)SONY_ASCOT3_DTV_DTMB);     /* DTMB */

    GetALine(strInput, sizeof(strInput));

    system = (sony_ascot3_tv_system_t)strtoul(strInput, NULL, 0);

    if(!SONY_ASCOT3_IS_ATV(system) && !SONY_ASCOT3_IS_DTV(system)){
        printf("Invalid system.\n");
        return SONY_RESULT_OK;
    }

    result = sony_ascot3_Tune(&g_tuner, frequency, system);
    if(result == SONY_RESULT_OK){
        SONY_SLEEP(50);
        result = sony_ascot3_TuneEnd(&g_tuner);
    }

    PrintResult(result);
    return result;
}

static sony_result_t ShiftFRF(void)
{
    sony_result_t result = SONY_RESULT_OK;
    char strInput[16];

    uint32_t frequency = 0;

    /* Frequency */
    printf("Current Frequency: %f MHz\n", (double)g_tuner.frequencykHz / 1000.0);
    printf("Input Frequency(MHz)\n");
    GetALine(strInput, sizeof(strInput));
    frequency = (uint32_t)(atof(strInput) * 1000.0);

    result = sony_ascot3_ShiftFRF(&g_tuner, frequency);

    PrintResult(result);
    return result;
}

static sony_result_t TunerSleep(void)
{
    sony_result_t result = SONY_RESULT_OK;

    result = sony_ascot3_Sleep(&g_tuner);
    if(result != SONY_RESULT_OK){
        PrintResult(result);
        return result;
    }

    return result;
}

static sony_result_t SetTunerGPO(void){
    sony_result_t result = SONY_RESULT_OK;
    char strInput[16];
    uint8_t id = 0;
    uint8_t val = 0;

    printf("Input GPO ID. (0/1)\n");
    GetALine(strInput, sizeof(strInput));
    switch(strInput[0]){
    case '0':
        id = 0; break;
    case '1':
        id = 1; break;
    default:
        printf("Invalid ID.\n"); return SONY_RESULT_OK;
    }
    printf("Input value. (0/1)\n");
    GetALine(strInput, sizeof(strInput));
    switch(strInput[0]){
    case '0':
        val = 0; break;
    case '1':
        val = 1; break;
    default:
        printf("Invalid value.\n"); return SONY_RESULT_OK;
    }
    printf("GPO %d = %d\n", id, val);

    result = sony_ascot3_SetGPO(&g_tuner, id, val);
    if(result != SONY_RESULT_OK){
        PrintResult(result);
        return result;
    }

    return SONY_RESULT_OK;
}

static sony_result_t RFFilterConfig(void)
{
    sony_result_t result = SONY_RESULT_OK;
    char strInput[16];
    uint8_t coeff = 0;
    uint8_t offset = 0;

    printf("Input VL_TRCKOUT_COEFF(multiplier). (8bit hex, Default: 0x80)\n");
    GetALine(strInput, sizeof(strInput));
    coeff = (uint8_t)strtoul(strInput, NULL, 16);
    printf("Input VL_TRCKOUT_OFS(additional term). (8bit hex, Default: 0x00)\n");
    GetALine(strInput, sizeof(strInput));
    offset = (uint8_t)strtoul(strInput, NULL, 16);
    printf("VL_TRCKOUT_COEFF: 0x%02X, VL_TRCKOUT_OFS: 0x%02X\n", coeff, offset);

    result = sony_ascot3_RFFilterConfig(&g_tuner, coeff, offset);
    if(result != SONY_RESULT_OK){
        PrintResult(result);
        return result;
    }

    return SONY_RESULT_OK;
}

static sony_result_t ReadRssi(void)
{
    sony_result_t result = SONY_RESULT_OK;
    int32_t rssi = 0;

    result = sony_ascot3_ReadRssi(&g_tuner, &rssi);
    if(result != SONY_RESULT_OK){
        PrintResult(result);
        return result;
    }

    printf("RSSI: %.02f\n", rssi/100.0);

    return SONY_RESULT_OK;
}

static sony_result_t TunerDump(void)
{
    /* Dump ALL register */
    /* This function only support register type access like ASCOT */
    uint8_t data[256];
    sony_result_t result = SONY_RESULT_OK;

    result = g_tuner.pI2c->ReadRegister(g_tuner.pI2c, g_tuner.i2cAddress, 0x00, data, sizeof(data));
    if(result != SONY_RESULT_OK){
        PrintResult(result);
        return result;
    }

    printf("--- Tuner register ---\n");
    PrintDumpData(data, sizeof(data));

    return result;
}

static sony_result_t ToggleTrace(void)
{
    g_traceEnable = g_traceEnable ? 0 : 1;
    if(g_traceEnable){
        printf("Trace Enabled\n");
    }else{
        printf("Trace Disabled\n");
    }
    return SONY_RESULT_OK;
}

static sony_result_t ToggleI2cLog(void)
{
    if(!g_i2cLogFile){
        char filename[64];
        time_t ltime;
        struct tm *tm;

        time(&ltime);
        tm = localtime(&ltime);
        sprintf(filename, "i2clog_%04d%02d%02d%02d%02d%02d.log",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

        g_i2cLogFile = fopen(filename, "w");
        if(!g_i2cLogFile){
            printf("fopen() failed...\n");
            return SONY_RESULT_ERROR_OTHER;
        }

        sony_i2c_EnableI2cLog(&g_i2cTunerLog, g_i2cLogFile);

        printf("I2c Log Enabled (%s)\n", filename);
    }else{
        fclose(g_i2cLogFile);
        g_i2cLogFile = NULL;
        sony_i2c_EnableI2cLog(&g_i2cTunerLog, NULL);

        printf("I2c Log Disabled\n");
    }

    return SONY_RESULT_OK;
}

static sony_result_t Exit(void)
{
    I2C_FINALIZE();
    FinalizeI2cLog();
    exit(0);
}

static void FinalizeI2cLog(void)
{
    if(g_i2cLogFile){
        fclose(g_i2cLogFile);
        g_i2cLogFile = NULL;
    }
}

/*------------------------------------------------------------------------------
  Implementation of Static functions (Utility)
------------------------------------------------------------------------------*/
/* Utility functions */
static void GetALine(char* pStr, int nSize)
{
    int nRead = 0;
    char ch = 0;

    if((!pStr) || (nSize <= 0)){
        return;
    }

    while((ch = (char)getchar()) != '\n'){
        if(nRead < nSize){
            pStr[nRead] = ch;
            nRead++;
        }
    }
    /* Insert last '\0' */
    if(nRead < nSize){
        pStr[nRead] = '\0';
    }else{
        pStr[nSize-1] = '\0';
    }
}

static void PrintDumpData(unsigned char *pData, int nSize)
{
    int i = 0;
    printf("       +0 +1 +2 +3 +4 +5 +6 +7 +8 +9 +A +B +C +D +E +F\n");
    for(i=0; i<nSize; i++){
        if(i%16 == 0){
            printf("%04x | ", i);
        }else{
            printf(" ");
        }
        printf("%02x", pData[i]);
        if((i%16 == 15) && (i != nSize-1)){
            printf("\n");
        }
    }
    printf("\n");
}

static void PrintResult(sony_result_t result)
{
    switch(result)
    {
    case SONY_RESULT_OK:
        printf("Success.\n");
        break;
    case SONY_RESULT_ERROR_ARG:
        printf("ERROR: Invalid argument. (maybe software bug)\n");
        break;
    case SONY_RESULT_ERROR_I2C:
        printf("ERROR: I2C communication error.\n");
        break;
    case SONY_RESULT_ERROR_SW_STATE:
        printf("ERROR: Invalid software state.\n");
        break;
    case SONY_RESULT_ERROR_HW_STATE:
        printf("ERROR: Invalid hardware state.\n");
        break;
    case SONY_RESULT_ERROR_TIMEOUT:
        printf("ERROR: Timeout occured.\n");
        break;
    case SONY_RESULT_ERROR_UNLOCK:
        printf("ERROR: Failed to lock.\n");
        break;
    case SONY_RESULT_ERROR_RANGE:
        printf("ERROR: Out of range.\n");
        break;
    case SONY_RESULT_ERROR_NOSUPPORT:
        printf("ERROR: This parameter is not supported.\n");
        break;
    case SONY_RESULT_ERROR_CANCEL:
        printf("ERROR: The operation is canceled.\n");
        break;
    case SONY_RESULT_ERROR_OTHER:
        printf("ERROR: Other Error.\n");
        break;
    default:
        printf("Unknown result code.\n");
        break;
    }
}

/*------------------------------------------------------------------------------
  Sample implementation of trace log function
  These functions are declared in sony_common.h
  NOTE: SONY_TRACE_ENABLE macro is defined in project file as a compile option.
------------------------------------------------------------------------------*/
static const char* g_callStack[256]; /* To output function name in TRACE_RETURN log */
static int g_callStackTop = 0;

void sony_trace_log_enter(const char* funcname, const char* filename, unsigned int linenum)
{
    if(!funcname || !filename){
        return;
    }

    if(g_traceEnable){
        /*
          <PORTING>
          NOTE: strrchr(filename, '\\')+1 can get file name from full path string.
                If in linux system, '/' will be used instead of '\\'.
        */
        const char* pFileNameTop = NULL;
        pFileNameTop = strrchr(filename, '\\');
        if(pFileNameTop){
            pFileNameTop += 1;
        }else{
            pFileNameTop = filename; /* Cannot find '\\' */
        }

        printf("TRACE_ENTER: %s (%s, line %d)\n", funcname, pFileNameTop, linenum);
    }
    if(g_callStackTop >= sizeof(g_callStack)/sizeof(g_callStack[0]) - 1){
        printf("ERROR: Call Stack Overflow...\n");
        return;
    }
    g_callStack[g_callStackTop] = funcname;
    g_callStackTop++;
}

void sony_trace_log_return(sony_result_t result, const char* filename, unsigned int linenum)
{
    const char* pErrorName = NULL;
    const char* pFuncName = NULL;
    const char* pFileNameTop = NULL;

    if(g_callStackTop > 0){
        g_callStackTop--;
        pFuncName = g_callStack[g_callStackTop];
    }else{
        printf("ERROR: Call Stack Underflow...\n");
        pFuncName = "Unknown Func";
    }

    switch(result)
    {
    case SONY_RESULT_OK:
    case SONY_RESULT_ERROR_TIMEOUT:
    case SONY_RESULT_ERROR_UNLOCK:
    case SONY_RESULT_ERROR_CANCEL:
        /* Only print log if FATAL error is occured */
        if(!g_traceEnable){
            return;
        }
        break;
    default:
        break;
    }

    switch(result)
    {
    case SONY_RESULT_OK:
        pErrorName = "OK";
        break;
    case SONY_RESULT_ERROR_TIMEOUT:
        pErrorName = "ERROR_TIMEOUT";
        break;
    case SONY_RESULT_ERROR_UNLOCK:
        pErrorName = "ERROR_UNLOCK";
        break;
    case SONY_RESULT_ERROR_CANCEL:
        pErrorName = "ERROR_CANCEL";
        break;
    case SONY_RESULT_ERROR_ARG:
        pErrorName = "ERROR_ARG";
        break;
    case SONY_RESULT_ERROR_I2C:
        pErrorName = "ERROR_I2C";
        break;
    case SONY_RESULT_ERROR_SW_STATE:
        pErrorName = "ERROR_SW_STATE";
        break;
    case SONY_RESULT_ERROR_HW_STATE:
        pErrorName = "ERROR_HW_STATE";
        break;
    case SONY_RESULT_ERROR_RANGE:
        pErrorName = "ERROR_RANGE";
        break;
    case SONY_RESULT_ERROR_NOSUPPORT:
        pErrorName = "ERROR_NOSUPPORT";
        break;
    case SONY_RESULT_ERROR_OTHER:
        pErrorName = "ERROR_OTHER";
        break;
    default:
        pErrorName = "ERROR_UNKNOWN";
        break;
    }

    pFileNameTop = strrchr(filename, '\\');
    if(pFileNameTop){
        pFileNameTop += 1;
    }else{
        pFileNameTop = filename; /* Cannot find '\\' */
    }

    printf("TRACE_RETURN: %s (%s) (%s, line %d)\n", pErrorName, pFuncName, pFileNameTop, linenum);
}
#endif
