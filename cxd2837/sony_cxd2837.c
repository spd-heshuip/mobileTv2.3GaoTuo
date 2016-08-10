#if 0

/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-04-18 02:10:28 #$
  File Revision : $Revision:: 5034 $
------------------------------------------------------------------------------*/
/**
 @file    sony_example_dvbt_tune_monitor.c

         This file provides an example of DVB-T tuning and looped monitoring
*/
/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "sony_integ.h"                 /* Integration layer API */
#include "sony_demod_driver_version.h"  /* Driver version file */
#include "sony_demod.h"                 /* Demodulator layer API and definitions */
#include "sony_demod_dvbt.h"            /* DVBT type definitions and demodulator functions */
#include "sony_integ_dvbt_t2.h"         /* DVBT integration layer functions (Tune, Scan, RF Level monitor etc)*/
#include "sony_demod_dvbt_monitor.h"    /* DVBT monitor functions */
//#include "drvi2c_feusb.h"               /* USB->I2C driver implementation. */
//#include "sony_ascot2d.h"               /* Sony ASCOT2D tuner driver core. */
//#include "sony_tuner_ascot2d.h"         /* Sony ASCOT2D tuner driver wrapper. */

#ifdef __cplusplus
}
#endif

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
/* Tuner Configuration */
//#define TUNER_SONY_ASCOT2D         /**< Define for Sony Ascot2D tuner. */
//#define TUNER_IFAGCPOS             /**< Define for IFAGC sense positive. */
//#define TUNER_SPECTRUM_INV         /**< Define for spectrum inversion. */
#define TUNER_RFLVLMON_DISABLE     /**< Define to disable RF level monitoring. */

#define SONY_TUNER_OFFSET_CUTOFF_HZ 100000

/* Comment out definitions below to disable monitor output */
#define MONITOR_SYNCSTAT            
#define MONITOR_IFAGCOUT
#define MONITOR_MODEGUARD
#define MONITOR_CARRIEROFFSET
#define MONITOR_PREVITERBIBER
#define MONITOR_PRERSBER
#define MONITOR_TPSINFO
#define MONITOR_PACKETERRORNUMBER
#define MONITOR_SPECTRUMSENSE
#define MONITOR_SNR
#define MONITOR_PPMOFFSET
#define MONITOR_QUALITY
#define MONITOR_PER

#define MONITOR_LOOP_COUNT  1   /* Number of times to run the channel monitors */

/*------------------------------------------------------------------------------
 Const char definitions
------------------------------------------------------------------------------*/
static const char *Common_DemodXtal[] = { "16MHz", "20.5MHz", "24MHz", "41MHz"};
static const char *Common_Bandwidth[] = { "Unknown", "1.7MHz", "Invalid", "Invalid", "Invalid", "5MHz", "6MHz", "7MHz", "8MHz"};
static const char *Common_TunerOptimize[] = { "Unknown", "ASCOT2D", "ASCOT2E"};
static const char *Common_SpectrumSense[] = { "Normal", "Inverted"};
static const char *Common_Result[] = { "OK", "Argument Error", "I2C Error", "SW State Error", "HW State Error", "Timeout", "Unlock", "Out of Range", "No Support", "Cancelled", "Other Error", "Overflow", "OK - Confirm"};
static const char *Common_YesNo[] = { "No", "Yes" };

static const char *DVBT_Constellation[] = { "QPSK", "16-QAM", "64-QAM" };
static const char *DVBT_Hier[] = { "Non", "A1", "A2", "A4" };
static const char *DVBT_CodeRate[] = { "1/2", "2/3", "3/4", "5/6", "7/8" };
static const char *DVBT_GI[] = { "1/32", "1/16", "1/8", "1/4" };
static const char *DVBT_Mode[] = { "2K", "8K" };
static const char *DVBT_Profile[] = { "HP", "LP" };

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
int main (int argc, char *argv[])
{
    sony_result_t result = SONY_RESULT_OK;
    sony_result_t tuneResult = SONY_RESULT_OK;
    
    sony_integ_t integ;
    sony_demod_t demod;
    sony_tuner_terr_cable_t tunerTerrCable;
//    sony_ascot2d_t ascot2d;

    sony_i2c_t i2c;
	sony_i2c_t tunerI2C;
    sony_dvbt_tune_param_t tuneParam;
 //   drvi2c_feusb_t feusb;
    uint8_t i = 0;

    argc = argc;
    argv = argv;

	Port_Init();
    /*------------------------------------------------------------------------------
     Setup / Initialisation
    ------------------------------------------------------------------------------*/
    /* Create I2C interface for tuner and demosulator parts.  GW (GateWay) members 
       provided but not required for tuner communication.  I2C switching handled 
       internally in the driver using a repeater. */
    i2c.gwAddress = 0x00;                                   /* N/A */
    i2c.gwSub = 0x00;                                       /* N/A */
    i2c.Read = Cxd2837_I2c_Read;                           /* Base level HW interfacing I2C read function */
    i2c.Write = Cxd2837_I2c_Write;                         /* Base level HW interfacing I2C write function */
    i2c.ReadRegister = sony_i2c_CommonReadRegister;         /* Common wrapper function for multi byte Read operation */
    i2c.WriteRegister = sony_i2c_CommonWriteRegister;       /* Common wrapper function for multi byte Write operation */
    i2c.WriteOneRegister = sony_i2c_CommonWriteOneRegister; /* Common wrapper function for single byte Write operation */

	    /* Setup I2C interfaces. */
    tunerI2C.gwAddress = 0xc8;
    tunerI2C.gwSub = 0x09;      /* Connected via demod I2C gateway function. */
    tunerI2C.Read = Cxd2837_I2c_ReadGw;
    tunerI2C.Write = Cxd2837_I2c_WriteGw;
    tunerI2C.ReadRegister = sony_i2c_CommonReadRegister;
    tunerI2C.WriteRegister = sony_i2c_CommonWriteRegister;
    tunerI2C.WriteOneRegister = sony_i2c_CommonWriteOneRegister;
  

    /* Display driver credentials */
    printf ("Driver Version : %s\n", SONY_DEMOD_DRIVER_VERSION);
    printf ("Built          : %s %s\n\n", SONY_DEMOD_DRIVER_RELEASE_DATE, SONY_DEMOD_DRIVER_RELEASE_TIME);

    printf ("------------------------------------------\n");
    printf (" Create / Inititialize   \n");
    printf ("------------------------------------------\n");
    /* Create ASCOT2D tuner with the following parameters, please modify as appropriate :
     *  - XTal = 41MHz
     *  - Address = 0xC0
     *  - Configuration Flags = None
     */
    {
        uint8_t xtalFreqMHz = 41;
        uint8_t i2cAddress = 0xc2;
        uint32_t configFlags = 0;

		result =  mopll_tuner_Create (i2cAddress,xtalFreqMHz,&tunerI2C, 0, NULL, &tunerTerrCable);

        if (result == SONY_RESULT_OK) {
            printf (" Tuner Created with the following parameters:\n");
            printf ("  - Tuner Type     : CXD2837 \n");
            printf ("  - XTal Frequency : %uMHz\n", xtalFreqMHz);
            printf ("  - I2C Address    : %u\n", i2cAddress);
            printf ("  - Config Flags   : %u\n\n", configFlags);
        }
        else {
            printf (" Error: Unable to create Sony ASCOT2D tuner driver. (result = %s)\n", Common_Result[result]);
            return -1;
        }
    }

    /* Create the integration structure which contains the demodulaor and tuner part instances.  This 
     * function also internally Creates the demodulator part.  Once created the driver is in 
     * SONY_DEMOD_STATE_INVALID and must be initialized before calling a Tune / Scan or Monitor API. 
     */
    {
        /* Modify the following to suit your implementation */
        sony_demod_xtal_t xtalFreq = SONY_DEMOD_XTAL_41000KHz;
        uint8_t i2cAddress = 0xc8;

        /* Create parameters for integration structure:
         *  sony_integ_t * pInteg                       Integration object
         *  sony_demod_xtal_t xtalFreq                  Demodulator xTal frequency
         *  uint8_t i2cAddress                          Demodulator I2C address
         *  sony_i2c_t i2c                              Demodulator I2C driver
         *  sony_demod_t *pDemod                        Demodulator object
         *
         *  Note: Set the following to NULL to disable control
         *  sony_tuner_terr_cable_t * pTunerTerrCable   Terrestrial / Cable tuner object 
         *  sony_tuner_sat_t * pTunerSat                Satellite tuner object
         *  sony_lnbc_t * pLnbc                         LNB Controller object
         */
        result = sony_integ_Create (&integ, xtalFreq, i2cAddress, &i2c, &demod
#ifdef SONY_DEMOD_SUPPORT_TERR_OR_CABLE
            /* Terrestrial and Cable supported so include the tuner object into the Create API */
            ,&tunerTerrCable
#endif
#ifdef SONY_DEMOD_SUPPORT_DVBS_S2
            /* Satellite supported so include the tuner and LNB objects into the Create API */
            , NULL, NULL
#endif
            );

        if (result == SONY_RESULT_OK) {
            printf (" Demod Created with the following parameters:\n");
            printf ("  - XTal Frequency : %s\n", Common_DemodXtal[xtalFreq]);
            printf ("  - I2C Address    : %u\n\n", i2cAddress);
        }
        else {
            printf (" Error: Unable to create demodulator driver. (result = %s)\n", Common_Result[result]);
            return -1;
        }
    }

    /* Initialize the tuner and demodulator parts to Terrestrial / Cable mode.  Following this call the
     * driver will be in SONY_DEMOD_STATE_SLEEP_T_C state. From here you can call any Shutdown, Sleep or Tune
     * API for Terrestrial / Cable systems or call sony_integ_SleepS to transfer to Satellite mode. 
     * 
     * Note : Initialize API should only be called once at the start of the driver creation.  Subsequent calls
     *        to any of the Tune API's do not require re-initialize. 
     */
    result = sony_integ_InitializeT_C (&integ);
    if (result == SONY_RESULT_OK) {
        printf (" Driver initialized, current state = SONY_DEMOD_STATE_SLEEP_T_C\n\n");
    }
    else {
        printf (" Error: Unable to initialise the integration driver to terrestiral / cable mode. (result = %s)\n", Common_Result[result]);
        return -1;
    }

    /* ---------------------------------------------------------------------------------
     * Configure the Demodulator
     * ------------------------------------------------------------------------------ */
    /* DVB-T demodulator IF configuration  Low IF tuner */
    demod.iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (36.167);
    demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (36.167);
    demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (36.167);
    demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (36.167);

    /* DVB-T2 demodulator IF configuration  Low IF tuner */
    demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (36.167);
    demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (36.167);
    demod.iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (36.167);
    demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (36.167);

    /* DVB-C demodulator IF configuration  Low IF tuner */
    demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (36.167);

    /* DVB-C2 demodulator IF configuration  Low IF tuner */
    demod.iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (36.167);
    demod.iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (36.167);

    /* IFAGC setup. Modify to suit connected tuner. */
    /* IFAGC positive, value = 0. */
#ifdef TUNER_IFAGCPOS
    result = sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_IFAGCNEG, 0);
    if (result == SONY_RESULT_OK) {
        printf (" Demodulator configured for Negative IFAGC.\n");
    }
    else {
        printf (" Error: Unable to configure for Negative IFAGC. (result = %s)\n", Common_Result[result]);
        return -1;
    }
#endif

    /* Spectrum Inversion setup. Modify to suit connected tuner. */
    /* Spectrum inverted, value = 1. */
#ifdef TUNER_SPECTRUM_INV
    result = sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_SPECTRUM_INV, 1);
    if (result == SONY_RESULT_OK) {
        printf (" Demodulator configured for Inverted Spectrum.\n");
    }
    else {
        printf (" Error: Unable to configure SPECTRUM_INV. (result = %s)\n", Common_Result[result]);
        return -1;
    }
#endif

    /* RF level monitoring (RFAIN/RFAGC) enable/disable. */
    /* Default is enabled. 1: Enable, 0: Disable. */
#ifdef TUNER_RFLVLMON_DISABLE
    result = sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_RFLVMON_ENABLE, 0);
    if (result == SONY_RESULT_OK) {
        printf (" Demodulator configured to enable RF level monitoring.\n");
    }
    else {
        printf (" Error: Unable to configure RFLVMON_ENABLE. (result = %s)\n", Common_Result[result]);
        return -1;
    }
#endif

    /* Demod tunerOptimize member allows the demod to be optimized internally when
       connected to Sony RF parts.  Please ensure this is set to 
       SONY_DEMOD_TUNER_OPTIMIZE_UNKNOWN for all other tuners */
#ifdef TUNER_SONY_ASCOT2D
    demod.tunerOptimize = SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2D;
#else
    demod.tunerOptimize = SONY_DEMOD_TUNER_OPTIMIZE_UNKNOWN;
#endif

    printf (" Demodulator optimised for %s tuner.\n\n", Common_TunerOptimize[demod.tunerOptimize]);

    /* ---------------------------------------------------------------------------------
     * Tune
     * ------------------------------------------------------------------------------ */
    printf ("------------------------------------------\n");
    printf (" Tune   \n");
    printf ("------------------------------------------\n");

    /* Configure the DVBT tune parameters based on the channel requirements */
    tuneParam.bandwidth = SONY_DEMOD_BW_8_MHZ;          /* Channel bandwidth */
    tuneParam.centerFreqKHz = 634000;                   /* Channel centre frequency in KHz */
    tuneParam.profile = SONY_DVBT_PROFILE_HP;           /* Channel profile for hierachical modes.  For non-hierachical use HP */

    printf (" Tune to DVB-T signal with the following parameters:\n");
    printf ("  - Center Freq    : %uKHz\n", tuneParam.centerFreqKHz);
    printf ("  - Bandwidth      : %s\n", Common_Bandwidth[tuneParam.bandwidth]);
    printf ("  - Profile        : %s\n", DVBT_Profile[tuneParam.profile]);

    /* Perform DVBT Tune */
    tuneResult = sony_integ_dvbt_Tune (&integ, &tuneParam);
    printf ("  - Result         : %s\n\n", Common_Result[tuneResult]);
    /* ---------------------------------------------------------------------------------
     * Carrier Offset Compensation
     * ------------------------------------------------------------------------------ */
    /* Measure the current carrier offset and retune to compensate for cases outside the demodulator
     * acquisition range. */
    if ((tuneResult == SONY_RESULT_ERROR_TIMEOUT) || (tuneResult == SONY_RESULT_OK)) {
        int32_t offsetHz = 0;
        uint32_t stepHz = SONY_TUNER_OFFSET_CUTOFF_HZ;

        /* Monitor carrier offset. */
        result = sony_demod_dvbt_monitor_CarrierOffset (integ.pDemod, &offsetHz);
        if (result != SONY_RESULT_OK) {
            printf ("Error: Unable to monitor T carrier offset. (result = %s)\n", Common_Result[result]);
            return -1;
        }

        printf (" DVB-T carrier offset of %ldHz detected.\n", offsetHz);

        /* Carrier recovery loop locked (demod locked), compensate for the offset and retry tuning. */
        stepHz = (stepHz + 1) / 2;
        if ((uint32_t) abs (offsetHz) > stepHz) {
            /* Tuners have only a fixed frequency step size (stepkHz), therefore we must query the tuner driver to get the actual
             * center frequency set by the tuner. */
            tuneParam.centerFreqKHz = (uint32_t) ((int32_t) integ.pTunerTerrCable->frequencyKHz + ((offsetHz + 500) / 1000));

            printf (" Re-tuning to compensate offset. New parameters:\n");
            printf ("  - Center Freq    : %uKHz\n", tuneParam.centerFreqKHz);
            printf ("  - Bandwidth      : %s\n", Common_Bandwidth[tuneParam.bandwidth]);
            printf ("  - Profile        : %s\n", DVBT_Profile[tuneParam.profile]);
            tuneResult = sony_integ_dvbt_Tune (&integ, &tuneParam);
            printf ("  - Result         : %s\n\n", Common_Result[tuneResult]);
        }
        else {
            printf (" Carrier offset compensation not required.\n");
        }
    }
    if (tuneResult != SONY_RESULT_OK) {
        printf (" Error: Unable to get TS lock DVB-T signal at %lukHz. (status=%d, %s)\n", tuneParam.centerFreqKHz, tuneResult, Common_Result[result]);
        return -1;
    }

    printf (" TS locked to DVB-T signal at %lukHz.\n\n", tuneParam.centerFreqKHz);

    /* ---------------------------------------------------------------------------------
     * Monitor the Channel
     * ------------------------------------------------------------------------------ */
    printf ("------------------------------------------\n");
    printf (" Monitor   \n");
    printf ("------------------------------------------\n");

    /* Allow the monitors time to settle */
    SONY_SLEEP (1000);

    for (i = 1; i <= MONITOR_LOOP_COUNT; i++) {

    printf ("\n Monitor Iteration : %u \n\n",i);
    printf ("-------------------------|-----------------|----------------- \n");
    printf ("      MONITOR NAME       |    PARAMETER    |      VALUE       \n");
    printf ("-------------------------|-----------------|----------------- \n");

#ifdef MONITOR_SYNCSTAT        
        {
            uint8_t syncState = 0;
            uint8_t tsLock = 0;
            uint8_t earlyUnlock = 0;
            result = sony_demod_dvbt_monitor_SyncStat (integ.pDemod, &syncState, &tsLock, &earlyUnlock);
            if (result == SONY_RESULT_OK) {
                printf (" SyncStat                | SyncStat        | %lu\n", syncState);
                printf ("                         | TS Lock         | %s\n", Common_YesNo[tsLock]);
                printf ("                         | Early Unlock    | %s\n", Common_YesNo[earlyUnlock]);
            }
            else {
                printf (" SyncStat                | Error           | %s\n", Common_Result[result]);
            }    
            printf ("-------------------------|-----------------|----------------- \n");
        }
#endif

#ifdef MONITOR_IFAGCOUT
        {
            uint32_t ifAGCOut = 0;
            result = sony_demod_dvbt_monitor_IFAGCOut (integ.pDemod, &ifAGCOut);
            if (result == SONY_RESULT_OK) {
                printf (" IFAGCOut                | IF AGC          | %lu\n", ifAGCOut);
            }
            else {
                printf (" IFAGCOut                | Error           | %s\n", Common_Result[result]);
            }  
            printf ("-------------------------|-----------------|----------------- \n");
        }
#endif

#ifdef MONITOR_MODEGUARD
        {
            sony_dvbt_mode_t mode;
            sony_dvbt_guard_t guard;
            result = sony_demod_dvbt_monitor_ModeGuard (integ.pDemod, &mode, &guard);
            if (result == SONY_RESULT_OK) {
                printf (" ModeGuard               | Mode            | %s\n", DVBT_Mode[mode]);
                printf ("                         | Guard Interval  | %s\n", DVBT_GI[guard]);
            }
            else {
                printf (" ModeGuard               | Error           | %s\n", Common_Result[result]);
            }  
            printf ("-------------------------|-----------------|----------------- \n");
        }
#endif

#ifdef MONITOR_CARRIEROFFSET
        {
            int32_t offset;
            result = sony_demod_dvbt_monitor_CarrierOffset (integ.pDemod, &offset);
            if (result == SONY_RESULT_OK) {
                printf (" CarrierOffset           | Carrier Offset  | %dHz\n", offset);
            }
            else {
                printf (" CarrierOffset           | Error           | %s\n", Common_Result[result]);
            }  
            printf ("-------------------------|-----------------|----------------- \n");
        }
#endif

#ifdef MONITOR_PREVITERBIBER
        {
            uint32_t ber;
            result = sony_demod_dvbt_monitor_PreViterbiBER (integ.pDemod, &ber);
            if (result == SONY_RESULT_OK) {
                printf (" PreViterbiBER           | Pre-Viterbi BER | %u x 10^-7\n", ber);
            }
            else {
                printf (" PreViterbiBER           | Error           | %s\n", Common_Result[result]);
            }  
            printf ("-------------------------|-----------------|----------------- \n");
        }
#endif

#ifdef MONITOR_PRERSBER
        {
            uint32_t ber;
            result = sony_demod_dvbt_monitor_PreRSBER (integ.pDemod, &ber);
            if (result == SONY_RESULT_OK) {
                printf (" PreRSBER                | Pre-RS BER      | %u x 10^-7\n", ber);
            }
            else {
                printf (" PreRSBER                | Error           | %s\n", Common_Result[result]);
            }  
            printf ("-------------------------|-----------------|----------------- \n");
        }
#endif

#ifdef MONITOR_TPSINFO
        {
            sony_dvbt_tpsinfo_t tpsInfo;
            result = sony_demod_dvbt_monitor_TPSInfo (integ.pDemod, &tpsInfo);
            if (result == SONY_RESULT_OK) {
                printf (" TPSInfo                 | Constellation   | %s\n", DVBT_Constellation[tpsInfo.constellation]);
                printf ("                         | Hierachy        | %s\n", DVBT_Hier[tpsInfo.hierarchy]);
                printf ("                         | Code Rate HP    | %s\n", DVBT_CodeRate[tpsInfo.rateHP]);
                printf ("                         | Code Rate LP    | %s\n", DVBT_CodeRate[tpsInfo.rateLP]);
                printf ("                         | Guard Interval  | %s\n", DVBT_GI[tpsInfo.guard]);
                printf ("                         | Mode            | %s\n", DVBT_Mode[tpsInfo.mode]);
            }
            else {
                printf (" TPSInfo                 | Error           | %s\n", Common_Result[result]);
            }  
            printf ("-------------------------|-----------------|----------------- \n");
        }
#endif

#ifdef MONITOR_PACKETERRORNUMBER
        {
            uint32_t pen;
            result = sony_demod_dvbt_monitor_PacketErrorNumber (integ.pDemod, &pen);
            if (result == SONY_RESULT_OK) {
                printf (" PacketErrorNumber       | PEN             | %u\n", pen);
            }
            else {
                printf (" PacketErrorNumber       | Error           | %s\n", Common_Result[result]);
            }  
            printf ("-------------------------|-----------------|----------------- \n");
        }
#endif

#ifdef MONITOR_SPECTRUMSENSE
        {
            sony_demod_terr_cable_spectrum_sense_t sense;
            result = sony_demod_dvbt_monitor_SpectrumSense (integ.pDemod, &sense);
            if (result == SONY_RESULT_OK) {
                printf (" SpectrumSense           | Spectrum Sense  | %s\n", Common_SpectrumSense[sense]);
            }
            else {
                printf (" SpectrumSense           | Error           | %s\n", Common_Result[result]);
            }  
            printf ("-------------------------|-----------------|----------------- \n");
        }
#endif

#ifdef MONITOR_SNR
        {
            int32_t snr;
            result = sony_demod_dvbt_monitor_SNR (integ.pDemod, &snr);
            if (result == SONY_RESULT_OK) {
                printf (" SNR                     | SNR             | %ddB x 10^-3\n", snr);
            }
            else {
                printf (" SNR                     | Error           | %s\n", Common_Result[result]);
            }  
            printf ("-------------------------|-----------------|----------------- \n");
        }
#endif

#ifdef MONITOR_PPMOFFSET
        {
            int32_t ppmOffset;
            result = sony_demod_dvbt_monitor_SamplingOffset (integ.pDemod, &ppmOffset);
            if (result == SONY_RESULT_OK) {
                printf (" SamplingOffset          | PPM Offset      | %d\n", ppmOffset);
            }
            else {
                printf (" SamplingOffset          | Error           | %s\n", Common_Result[result]);
            }  
            printf ("-------------------------|-----------------|----------------- \n");
        }
#endif

#ifdef MONITOR_QUALITY
        {
            uint8_t quality;
            result = sony_demod_dvbt_monitor_Quality (integ.pDemod, &quality);
            if (result == SONY_RESULT_OK) {
                printf (" Quality                 | SQI             | %u\n", quality);
            }
            else {
                printf (" Quality                 | Error           | %s\n", Common_Result[result]);
            }  
            printf ("-------------------------|-----------------|----------------- \n");
        }
#endif

#ifdef MONITOR_PER
        {
            uint32_t per;
            result = sony_demod_dvbt_monitor_PER (integ.pDemod, &per);
            if (result == SONY_RESULT_OK) {
                printf (" PER                     | PER             | %u x 10^-6\n", per);
            }
            else {
                printf (" PER                     | Error           | %s\n", Common_Result[result]);
            }  
            printf ("-------------------------|-----------------|----------------- \n");
        }
#endif

        SONY_SLEEP (1000);
    }

#if 0
    /*------------------------------------------------------------------------------
     Closing / Shutdown
    ------------------------------------------------------------------------------*/
    /* Shutdown the demodulator and tuner parts, placing them into a low power mode.  After this
     * operation the driver will be in SONY_DEMOD_STATE_SHUTDOWN state.  From this state you can 
     * call sony_integ_SleepT_C to return to Terrestrial / Cable mode, or sony_integ_SleepS to
     * transition to Satellite operation.  Both Sleep API's will load the demodulator configuration
     * from memory to retain functionality from before Shutdown.  
     * 
     * Note : If sony_integ_InitializeT_C or sony_integ_InitializeS are called the configuration 
     * memory will be cleared and the demod will perform a SW reset of all device registers.
     * This method of returning from Shutdown state is not recommended. */
    printf ("\n------------------------------------------\n");
    printf (" Shutdown / Finalize                      \n");
    printf ("------------------------------------------\n");

    result = sony_integ_Shutdown (&integ);
    if (result == SONY_RESULT_OK) {
        printf (" Demodulator and tuner put into Shutdown state.\n");
    }
    else {
        printf (" Error: Unable to shutdown the integration driver. (result = %s)\n", Common_Result[result]);
        return -1;
    }

    /* Finalise the I2C */
    result = drvi2c_feusb_Finalize (&feusb);
    if (result == SONY_RESULT_OK) {
        printf (" I2C driver instance closed.\n");
    }
    else {
        printf (" Error: Unable to finalize FEUSB I2C driver. (result = %s)\n", Common_Result[result]);
        return -1;
    }
#endif

    return 0;
}
#endif