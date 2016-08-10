/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 2675 $
    $Author: mrushton $

</dev:header>

------------------------------------------------------------------------------*/
/**
 @file    cxd2837_mopll_tuner.c

      Implementation for an example MOPLL tuner.
*/
/*----------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/
#include "sony_demod.h"
#include "mopll_tuner.h"

extern unsigned char TunerHV,TunMode,TunerSMode,TunerTone;

/*------------------------------------------------------------------------------
 Internal defines
------------------------------------------------------------------------------*/

#define PLL_TIMEOUT         50  /* Timeout for PLL lock condition. */
#define PLL_POLL_INTERVAL   10  /* Polling interval of PLL lock condition. */
#define PLL_RFAGC_WAIT      70  /* Wait for RFAGC stabilisation. */

/* Local defines for tuner driver. */
#define DVBT_REF_FREQ_HZ    SONY_DVB_TUNER_MOPLL_DVBT_REF_FREQ_HZ
#define DVBC_REF_FREQ_HZ    SONY_DVB_TUNER_MOPLL_DVBC_REF_FREQ_HZ
#define FREQ_IF             ((uint32_t)(SONY_DVB_TUNER_MOPLL_IF_MHZ * 1000))

/*
 MOPLL oscillator cut-off frequencies.
*/
#define FREQ_VL_1           (83000 - FREQ_IF)   /* fosc=83MHz */
#define FREQ_VL_2           (121000 - FREQ_IF)  /* fosc=121MHz */
#define FREQ_VL_3           (141000 - FREQ_IF)  /* fosc=141MHz */
#define FREQ_VL_4           (166000 - FREQ_IF)  /* fosc=166MHz */
#define FREQ_VL_5           (182000 - FREQ_IF)  /* fosc=182MHz */
#define FREQ_VH_1           (286000 - FREQ_IF)  /* fosc=286MHz */
#define FREQ_VH_2           (386000 - FREQ_IF)  /* fosc=386MHz */
#define FREQ_VH_3           (446000 - FREQ_IF)  /* fosc=446MHz */
#define FREQ_VH_4           (466000 - FREQ_IF)  /* fosc=466MHz */
#define FREQ_UH_1           (506000 - FREQ_IF)  /* fosc=506MHz */
#define FREQ_UH_2           (761000 - FREQ_IF)  /* fosc=761MHz */
#define FREQ_UH_3           (846000 - FREQ_IF)  /* fosc=846MHz */
#define FREQ_UH_4           (905000 - FREQ_IF)  /* fosc=905MHz */

#define FREQ_VL_LO          FREQ_VL_1
#define FREQ_VL_HI          FREQ_VL_5
#define FREQ_VH_HI          FREQ_VH_1
#define FREQ_UH_HI          FREQ_UH_4

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

static sony_result_t cxd2837_WritePLL (sony_tuner_terr_cable_t * pTuner, const uint8_t * siTunerBytes, uint32_t size);
static sony_result_t cxd2837_WaitPLLLock (sony_tuner_terr_cable_t * pTuner);

sony_result_t cxd2837_mopll_tuner_Create (uint8_t i2cAddress,
                                      uint32_t xtalFreq,
                                      sony_i2c_t * pI2c, uint32_t configFlags, void *user, sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
printf("[cxd2837_mopll_tuner_Create]--1\n");
    SONY_TRACE_ENTER ("cxd2837_mopll_tuner_Create");

    if ((pTuner == NULL) || (pI2c == NULL)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
printf("[cxd2837_mopll_tuner_Create]--2\n");
    pTuner->Initialize = cxd2837_mopll_tuner_Initialize;
    pTuner->Shutdown = cxd2837_mopll_tuner_Finalize;
    pTuner->Sleep = cxd2837_mopll_tuner_Sleep;
    pTuner->Tune = cxd2837_mopll_tuner_Tune;
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->bandwidth = SONY_DEMOD_BW_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->i2cAddress = i2cAddress;
//    pTuner->xtalFreq = xtalFreq;
    pTuner->pI2c = pI2c;
    pTuner->flags = configFlags;    /* Unused */
    pTuner->user = user;
printf("[cxd2837_mopll_tuner_Create]--3\n");
    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Initialise the tuner. No operation required.
------------------------------------------------------------------------------*/
sony_result_t cxd2837_mopll_tuner_Initialize (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("cxd2837_mopll_tuner_Initialize");
    /* Removes compiler warnings. */
    pTuner = pTuner;
 switch(TunMode){
#if 0
		case 21:
			//NM13x
			break;
		case 4://6651
			break;
		case 12:
			break;
		case 20:
			TDA18273_Init();
			break;
		case 29:
			//devTunerInit();
			break;
#endif
		case 27:	
		case 31://mxl603
		case 34:
		case 32:
		case 35:
		case 28:
		case 38:
		case 42:	
		case 44:
		case 45:
		case 46:	
			break;
#if 0
		case 36:
			TDA18250B_Init();
			break;
		case 26:
			R820C_init();
			break;
		case 41:
			TDA18275_Init();
			break;
#endif		
		default:
			SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
			break;
			
	}
    SONY_TRACE_RETURN (result);
}

sony_result_t cxd2837_mopll_tuner_Finalize (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("cxd2837_mopll_tuner_Finalize");
    /* Removes compiler warnings. */
    pTuner = pTuner;
    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Setup the channel bandwidth.
 Tune to the specified frequency.
 Wait for lock.
struct sony_tuner_terr_cable_t * pTuner,
                           uint32_t centerFrequencyKHz,
                           sony_dtv_system_t system,
                           sony_demod_bandwidth_t bandwidth
------------------------------------------------------------------------------*/
sony_result_t cxd2837_mopll_tuner_Tune (sony_tuner_terr_cable_t * pTuner, uint32_t frequency, sony_dtv_system_t system, sony_demod_bandwidth_t bandWidth)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t data[5];
	uint32_t n;
    SONY_TRACE_ENTER ("cxd2837_mopll_tuner_Tune");

    if (!pTuner) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Set current tuning parameter to struct */
    pTuner->system = system;
    pTuner->bandwidth = bandWidth;
pTuner->frequencyKHz =  frequency;
//printf("cxd2837_mopll_tuner_Tune;%d %d\n",frequency,system);
#if 0
	{
		if((TunerTone==0)||(TunerTone==1))
			TunMode=42;
		else
			TunMode=27;
	}	
#endif
printf("[cxd2837_mopll_tuner_Tune]--1,TunMode=%d\n",TunMode);
	switch(TunMode){
#if 0
		case 26:
			 R820C_Setfreq(frequency,(uint32_t)bandWidth*1000);
			break;
		case 41:
			TDA18275_Tune((unsigned long int)frequency*1000,(uint32_t)bandWidth*1000);
			break;
		case 36:
			TDA18250B_Tune((unsigned long int)frequency*1000,(uint32_t)bandWidth*1000);
			break;
		case 35:
			Si2148_SetPara(0xc0,TunerSMode,(unsigned long int)frequency*1000,(uint32_t)bandWidth*1000);
			break;
		case 38:
			Si2148_SetPara(0xc4,TunerSMode,(unsigned long int)frequency*1000,(uint32_t)bandWidth*1000);
			break;
		case 46:
			Si2144_SetPara(TunerSMode,(unsigned long int)frequency*1000,(uint32_t)bandWidth*1000);
			break;
		case 32:
		case 28:
			Si2158_SetPara(TunerSMode,(unsigned long int)frequency*1000,(uint32_t)bandWidth*1000);
		break;
		case 29:
			//devDigitalTuner_SetFreq(frequency,(uint32_t)bandWidth*1000);
			break;
		case 4://6651
						
			data[2] = (0xCA) ;    /* Mask, 166.667kHz XTOUT buffer off*/
			data[3] = (0x01) ;   /*VHL*/
			{     
				switch (system)
				{
				/* Intentional fall through. */
				case SONY_DTV_SYSTEM_DVBT:
				case SONY_DTV_SYSTEM_DVBT2:

						n = frequency;
						n += 36167;
						n *= 1000;
						n /= 16667;
						n += 5;
						n /= 10;
					break;
				case SONY_DTV_SYSTEM_DVBC:	
						data[2] = (0xC8) ;    /* Mask, 62.5kHz XTOUT buffer off*/
						n = frequency;
						n += 36125;
						n *= 1000;
						n /= 6250;
						n += 5;
						n /= 10;
					break;
			
				
				/* Intentional fall through. */
				case SONY_DTV_SYSTEM_UNKNOWN:
				default:
					SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
						break;
				}
			
				data[0] = (uint8_t)((n >> 8) & 0x7F) ;
				data[1] = (uint8_t)(n & 0xFF) ;
			}
			
			/* Setup RFAGC takeover point. 124dBuV  ATC=0*/
				data[4] = 0x80 ;
				
				/* Bandswitch */
				if (frequency > 444000) {
					data[3] = (0x04) ;
				}
				else if (frequency > 159000) {
					data[3] = (0x02) ;
				}
				else {
					data[3] = (0x01) ;
					//SONY_DVB_TRACE_RETURN (SONY_DVB_ERROR_RANGE);
				}
			
			
			/* Charge Pump */
				frequency += (uint32_t)SONY_DVB_TUNER_MOPLL_IF_MHZ*1000;
				if(((frequency>=80000)&&(frequency<92000))||((frequency>=196000)&&(frequency<224000)))
					data[3] |= 1<<5;
				else if(((frequency>=92000)&&(frequency<144000))||((frequency>=224000)&&(frequency<296000)))
					data[3] |= 2<<5;
				else if(((frequency>=144000)&&(frequency<156000))||((frequency>=296000)&&(frequency<380000))||((frequency>=484000)&&(frequency<604000)))
					data[3] |= 3<<5;
				else if(((frequency>=156000)&&(frequency<176000))||((frequency>=380000)&&(frequency<404000))||((frequency>=604000)&&(frequency<676000)))
					data[3] |= 7<<5;
				else if(((frequency>=176000)&&(frequency<184000))||((frequency>=404000)&&(frequency<448000))||((frequency>=676000)&&(frequency<752000)))
					data[3] |= 5<<5;
				else if(((frequency>=184000)&&(frequency<196000))||((frequency>=448000)&&(frequency<472000))||((frequency>=752000)&&(frequency<868000)))
					data[3] |= 6<<5;
			else //if(((frequency>=472000)&&(frequency<484000))||((frequency>=868000)&&(frequency<904000)))
					data[3] |= 7<<5;
				
			
	//printf("TDA651 data[3]:0x%x \n",data[3]);		
				data[3]&=0xe7;
				/* Setup bandwidth */
				switch (bandWidth)
				{
					/* Intentional fall through. */
					case 5:
					case 6:
						data[3] |= 0x10;
						break;
					case 7:
						data[3] &= 0xe7 ;
						break ;
					case 8: 
						data[3] |= (0x18) ;
						break ;
					default:
						SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
						break;
				}
			
			result = cxd2837_WritePLL (pTuner, data, 4);
			if (result != SONY_RESULT_OK) {
				SONY_TRACE_RETURN (result);
			}
			
			/* Wait for RFAGC stabilisation. */
			SONY_SLEEP (PLL_RFAGC_WAIT);
			
			/* Wait for PLL lock indication. */
			result = cxd2837_WaitPLLLock (pTuner);
			if (result != SONY_RESULT_OK) {
				SONY_TRACE_RETURN (result);
			}
			
			/* Re-send, but with the high RFAGC current OFF. */
			data[2] = data[4] ;
			result = cxd2837_WritePLL (pTuner, data, 4);
			if (result != SONY_RESULT_OK) {
				SONY_TRACE_RETURN (result);
			} 
			/* Wait for RFAGC stabilisation. */
			SONY_SLEEP (PLL_RFAGC_WAIT);
			
			/* Wait for PLL lock indication. */
			result = cxd2837_WaitPLLLock (pTuner);
			if (result != SONY_RESULT_OK) {
				SONY_TRACE_RETURN (result);
			}

			break;
		case 12:
			TDA18219_SetFreq(frequency*1000,(uint32_t)bandWidth*1000);
			if (result != SONY_RESULT_OK) {
     			   SONY_TRACE_RETURN (result);
  			  }
			break;
		case 20:
				/* Setup bandwidth */

//printf("bandWidth;;%d \n",bandWidth);

				switch (bandWidth)
				{
					/* Intentional fall through. */
					case SONY_DEMOD_BW_1_7_MHZ:
							n = 1700;
						break;
					case 5:
					case 6:
							n = 6000;
						break;
					case 7:
							n = 7000;
						break ;
					case 8: 
							n = 8000;
						break ;
					default:
						SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
						break;
				}
			TDA18273_SetFreq(frequency*1000,(uint32_t)n);
			break;
		case 21:
			NM131_SetFreq(frequency*1000,(uint32_t)bandWidth*1000);
			break;
		case 27:	
			TDA18250_SetRf((unsigned long int)frequency/1000,(uint32_t)bandWidth*1000);
			break;
		case 31:
		case 34:	
			Mxl603SetFreq(TunerSMode,(unsigned long int)frequency*1000,(uint32_t)bandWidth*1000);
			break;
		case 42:
			TDA18260_ProSetFreq(TunerTone,(unsigned long int)frequency);
			break;
		case 44:
			or53xx_SetFrequency(TunerSMode,(unsigned long int)frequency,(uint32_t)bandWidth*1000);
			break;
#endif
		case 45:
			sony_cxd2872__SetFreq(TunerSMode,(unsigned long int)frequency,(uint32_t)bandWidth*1000);
			break;
		default:
			SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
			break;
			
	}
printf("[cxd2837_mopll_tuner_Tune]\n");
    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Sleep the tuner module.
------------------------------------------------------------------------------*/
sony_result_t cxd2837_mopll_tuner_Sleep (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("cxd2837_mopll_tuner_Sleep");
    /* Removes compiler warnings. */
    pTuner = pTuner;
    SONY_TRACE_RETURN (result);
}

static sony_result_t cxd2837_WritePLL (sony_tuner_terr_cable_t * pTuner, const uint8_t * siTunerBytes, uint32_t size)
{
    return pTuner->pI2c->Write (pTuner->pI2c, pTuner->i2cAddress, (uint8_t *) siTunerBytes, size,
                                (SONY_I2C_START_EN | SONY_I2C_STOP_EN));
}

static sony_result_t cxd2837_WaitPLLLock (sony_tuner_terr_cable_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
    uint8_t i = 0;
    uint8_t data = 0;

    SONY_TRACE_ENTER ("cxd2837_WaitPLLLock");

    if (!pTuner) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    for (i = 0; i < PLL_TIMEOUT / PLL_POLL_INTERVAL; i++) {

        result = pTuner->pI2c->Read (pTuner->pI2c, pTuner->i2cAddress, &data, 1, (SONY_I2C_START_EN | SONY_I2C_STOP_EN));
        if (result != SONY_RESULT_OK) {
            break;
        }
//printf("data:%x\n",data);
        if (data & 0x40) {
            break;
        }
        result = SONY_RESULT_ERROR_TIMEOUT;
        SONY_SLEEP (PLL_POLL_INTERVAL);
    }
    SONY_TRACE_RETURN (result);
}
