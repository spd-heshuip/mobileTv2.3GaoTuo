

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
//#include <windows.h>
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
#include "sony_ascot2e.h"               /* Sony ASCOT2D tuner driver core. */
#include "../cxd2872/sony_ascot3.h"               /* Sony ASCOT2D tuner driver core. */
#include "sony_tuner_ascot2e.h"         /* Sony ASCOT2D tuner driver wrapper. */
#include "sony_demod_dvbt2.h"           /* DVBT2 type definitions and demodulator functions */
#include "sony_demod_dvbc.h"            /* DVBC type definitions and demodulator functions */
#include "sony_demod_dvbc2.h"           /* DVBC2 type definitions and demodulator functions */
#include "sony_integ_dvbc2.h"           /* DVBC2 integration layer functions (Tune, Scan, RF Level monitor etc)*/
#include "sony_demod_dvbc2_monitor.h"   /* DVBC2 monitor functions */
#ifdef __cplusplus
}
#endif
static sony_result_t sony_demod_dvbc_monitor_Quality (sony_demod_t * pDemod, uint8_t * pQuality);
static sony_result_t sony_integ_dvbc_monitor_SSI (sony_integ_t * pInteg, uint8_t * pSSI);

int drxk_drxj=0;
unsigned char cxd2837adr = 0xc8;
int CXD2837_DemodType = 0;
int cxd2837_bInit = 0;
int LockXtal;



/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
/* The following Quality monitor is provided as an example only as it is not based
 * on a formal specification. It is therefore advised that this is only used for
 * guidance, rather than an exact representation. 
 *
 * For improved accuracy the C/N ref table values can be characterised on your own
 * implementation.  Nordig specification values are currently used, but may not give
 * an accurate representation of your specific hardware capabilities.
 */
static sony_result_t sony_demod_dvbc_monitor_Quality (sony_demod_t * pDemod, uint8_t * pQuality)
{
    uint32_t ber = 0;
    int32_t snr = 0;
    int32_t snRel = 0;
    int32_t berSQI = 0;
    sony_dvbc_constellation_t constellation;

    /* Nordig spec C/N (Es/No) minimum performance 
     * Note: 32QAM isn't provided in the Nordig unified specification, so has been
     * Implemented based on interpolation and measurements. 
     */
    static const int32_t cnrNordigdB1000[] = {
    /*  16QAM   32QAM   64QAM   128QAM  256QAM */
        20000,  23000,  26000,  29000,  32000  };

    sony_result_t result = SONY_RESULT_OK;
    
    SONY_TRACE_ENTER ("sony_demod_dvbc_monitor_Quality");

    if ((!pDemod) || (!pQuality)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC) {
        /* Not DVB-C*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Get Pre-RS (Post-Viterbi) BER. */
    result = sony_demod_dvbc_monitor_PreRSBER (pDemod, &ber);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Get SNR value. */
    result = sony_demod_dvbc_monitor_SNR (pDemod, &snr);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }

    /* Get Code rate */
    result = sony_demod_dvbc_monitor_QAM (pDemod, &constellation);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN (result);
    }
       
    /* Calculate. */
    snRel = snr - cnrNordigdB1000[(uint8_t)constellation];
   
    /* BER_SQI is calculated from:
     * if (BER > 10^-3)          : 0
     * if (10^-7 < BER <= 10^-3) : BER_SQI = 20*log10(1/BER) - 40
     * if (BER <= 10^-7)         : BER_SQI = 100
      */
    if (ber > 10000) {
        berSQI = 0;
    }
    else if (ber > 1) {
        /* BER_SQI = 20 * log10(1/BER) - 40
         * BER_SQI = 20 * (log10(1) - log10(BER)) - 40
         * 
         * If BER in units of 1e-7
         * BER_SQI = 20 * (log10(1) - (log10(BER) - log10(1e7)) - 40
         * 
         * BER_SQI = 20 * (log10(1e7) - log10(BER)) - 40
         * BER_SQI = 20 * (7 - log10 (BER)) - 40
         */
        berSQI = (int32_t) (10 * sony_math_log10 (ber));
        berSQI = 20 * (7 * 1000 - (berSQI)) - 40 * 1000;
    }
    else {
        berSQI = 100 * 1000;
    }

    /* SQI (Signal Quality Indicator) given by:
     * if (C/Nrel < -7dB)         : SQI = 0
     * if (-7dB <= C/Nrel < +3dB) : SQI = (((C/Nrel - 3) / 10) + 1) * BER_SQI
     * if (C/Nrel >= +3dB)        : SQI = BER_SQI
     */
    if (snRel < -7 * 1000) {
        *pQuality = 0;
    }
    else if (snRel < 3 * 1000) {
        int32_t tmpSQI = (((snRel - (3 * 1000)) / 10) + 1000);
        *pQuality = (uint8_t) (((tmpSQI * berSQI) + (1000000/2)) / (1000000)) & 0xFF;
    }
    else {
        *pQuality = (uint8_t) ((berSQI + 500) / 1000);
    }

    /* Clip to 100% */
    if (*pQuality > 100) {
        *pQuality = 100;
    }

    SONY_TRACE_RETURN (result);
}
#if 0
/* The following SSI monitor is provided as an example only as it is not based
 * on a formal specification. It is therefore advised that this is only used for
 * guidance, rather than an exact representation. 
 *
 * For improved accuracy the C/N ref table values can be characterised on your own
 * implementation.  Nordig specification values are currently used, but may not give
 * an accurate representation of your specific hardware capabilities.
 *
 * NOTE : This function calls the integration layer RF Level monitor which is 
 * HW implementation dependant, therefore the SSI level may be incorrect RF Level 
 * is correctly characterised. */
static sony_result_t sony_integ_dvbc_monitor_SSI (sony_integ_t * pInteg, uint8_t * pSSI)
{
    sony_dvbc_constellation_t constellation;
    uint32_t symbolRate;
    int32_t prec;
    int32_t prel;
    int32_t pref;
    int32_t tempSSI = 0;  
    uint32_t noiseFigureDB1000;
    sony_result_t result = SONY_RESULT_OK;

    /* Nordig spec C/N (Es/No) minimum performance 
     * Note: 32QAM isn't provided in the Nordig unified specification, so has been
     * Implemented based on interpolation and measurements. 
     */
    static const int32_t cnrNordigdB1000[] = {
    /*  16QAM   32QAM   64QAM   128QAM  256QAM */
        20000,  23000,  26000,  29000,  32000  };

    SONY_TRACE_ENTER ("sony_integ_dvbc_monitor_SSI");

    if ((!pInteg) || (!pInteg->pDemod) || (!pSSI)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pInteg->pDemod->system != SONY_DTV_SYSTEM_DVBC) {
        /* Not DVB-C*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Get estimated RF Level */
    result = sony_integ_dvbc_monitor_RFLevel (pInteg, &prec);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Monitor constellation */
    result = sony_demod_dvbc_monitor_QAM (pInteg->pDemod, &constellation);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Monitor symbol rate */
    result = sony_demod_dvbc_monitor_SymbolRate(pInteg->pDemod, &symbolRate);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }    
    
    /* Modify this to suit the tuner noise figure specification value in dB * 1000 */
    noiseFigureDB1000 = 9000; 

    /* Reference sensitivity limit is calcualted from:
     * RefLevel (dB) = (10 * Log(1.38*10^-23 * 290)) + 30 + NoiseFigure + (10 * Log(symbolRate)) + C/N_Offset
     *  - sony_math_log10(x)     = 100 * Log(x)
     *  - Log(1.38*10^-23 * 290) = -20.3977
     *
     * Therefore:
     * RefLevel (dB * 1000) = -203977 + 30000 + (1000 * NoiseFigure) + (100 * Log(symbolRate)) + (1000 * C/N_Offset)
     */
    pref = -203977 + 30000 + noiseFigureDB1000 + (100 * sony_math_log10(symbolRate* 1000)) + cnrNordigdB1000[(uint8_t)constellation];

    /* prel = prec - pref */
    prel = prec - pref;

    /* SSI (Signal Strength Indicator) is calculated from:
     *
     * if (prel < -15dB)              SSI = 0
     * if (-15dB <= prel < 0dB)       SSI = (2/3) * (prel + 15)
     * if (0dB <= prel < 20dB)        SSI = 4 * prel + 10
     * if (20dB <= prel < 35dB)       SSI = (2/3) * (prel - 20) + 90
     * if (prel >= 35dB)              SSI = 100
     */
    if (prel < -15000) {
        tempSSI = 0;
    }
    else if (prel < 0) {
        /* Note : prel and 2/3 scaled by 10^3 so divide by 10^6 added */
        tempSSI = ((2 * (prel + 15000)) + 1500) / 3000;
    }
    else if (prel < 20000) {
        /* Note : prel scaled by 10^3 so divide by 10^3 added */
        tempSSI = (((4 * prel) + 500) / 1000) + 10;
    }
    else if (prel < 35000) {
        /* Note : prel and 2/3 scaled by 10^3 so divide by 10^6 added */
        tempSSI = (((2 * (prel - 20000)) + 1500) / 3000) + 90;
    }
    else {
        tempSSI = 100;
    }

    /* Clip value to 100% */
    *pSSI = (tempSSI > 100)? 100 : (uint8_t)tempSSI;

    SONY_TRACE_RETURN (result);
}

#endif


/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
/**
 @brief The following Quality monitor is provided as an example only as it is not based
        on a formal specification. It is therefore advised that this is only used for
        guidance, rather than an exact representation. 
 
        For improved accuracy the C/N ref table values can be characterised on your own
        implementation.  Nordig specification values are currently used, but may not give
        an accurate representation of your specific hardware capabilities.

 @param pDemod The demodulator instance.
 @param pQuality The quality as a percentage (0-100).

 @return SONY_RESULT_OK if successful and pQuality valid.
*/
static sony_result_t sony_demod_dvbc2_monitor_Quality (sony_demod_t * pDemod, uint8_t * pQuality)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_dvbc2_plp_code_rate_t codeRate;
    sony_dvbc2_constellation_t constellation;
    uint32_t ber = 0;
    int32_t snr = 0;
    int32_t snrRel = 0;
    int32_t berSQI = 0;

    /* The list of DVB-C2 SNR reciever performance requirement values in dBx1000. 
     * Entries equalling 0 indicate invalid code rate and constellation combination.
     */
    static const int32_t snrRefdB1000[6][7] = {
        /* RSVD1,  2/3,    3/4,    4/5,    5/6,    8/9,    9/10*/    
        {  0,      0,      0,      0,      0,      0,      0},     /* RSVD1 */
        {  0,      0,      0,      12900,  0,      15000,  15000}, /* 16-QAM */
        {  0,      15600,  0,      18200,  0,      20700,  20700}, /* 64-QAM */
        {  0,      0,      22200,  0,      24300,  26500,  26500}, /* 256-QAM */
        {  0,      0,      27300,  0,      30300,  33600,  33600}, /* 1024-QAM */
        {  0,      0,      0,      0,      39900,  41800,  41800}  /* 4096-QAM */
    };

    SONY_TRACE_ENTER ("sony_demod_dvbc2_monitor_Quality");

    if ((!pDemod) || (!pQuality)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }   

    /* Software state check */
    if (pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2 */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Get PLP Code Rate */
    result = sony_demod_dvbc2_monitor_CodeRate (pDemod, SONY_DVBC2_PLP_NORMAL_DATA, &codeRate);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Get PLP constellation */
    result = sony_demod_dvbc2_monitor_QAM (pDemod, SONY_DVBC2_PLP_NORMAL_DATA, &constellation);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }
    
    /* Get Pre-BCH (Post-LDPC) BER */
    result = sony_demod_dvbc2_monitor_PreBCHBER (pDemod, &ber);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Get SNR value */
    result = sony_demod_dvbc2_monitor_SNR(pDemod, &snr);
    if (result != SONY_RESULT_OK){
        SONY_TRACE_RETURN(result);
    }

    /* Ensure correct code rate and constellation values. */
    if ((codeRate >= SONY_DVBC2_R_RSVD2) || (constellation >= SONY_DVBC2_CONSTELLATION_RSVD2)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_OTHER);
    }

    /* BER_SQI Calculated from:
     * if (Pre-BCH BER > 10^-4)            BER_SQI = 0
     * if (10^-4 >= Pre-BCH BER >= 10^-7)  BER_SQI = 100/15 = 6.667
     * if (Pre-BCH BER < 10^-7)            BER_SQI = 100/6  = 16.667
     *
     * Note : Pre-BCH BER is scaled by 10^9
     */
    if (ber > 100000) {
        berSQI = 0;
    } 
    else if (ber >= 100) {
        berSQI = 6667;
    } 
    else {
        berSQI = 16667;
    }

    /* C/Nrel = C/Nrec - C/Nnordigp1 */
    snrRel = snr - snrRefdB1000[constellation][codeRate];

    /* SQI (Signal Quality Indicator) given by:
     * if (C/Nrel < -3dB)         SQI = 0
     * if (-3dB <= CNrel <= 3dB)  SQI = (C/Nrel + 3) * BER_SQI 
     * if (CNrel > 3dB)           SQI = 100
     */
    if (snrRel < -3000) {
        *pQuality = 0;
    } 
    else if (snrRel <= 3000) {
        /* snrRel and berSQI scaled by 10^3 so divide by 10^6 */
        uint32_t tempSQI = (((snrRel + 3000) * berSQI) + 500000) / 1000000;
        /* Clip value to 100% */
        *pQuality = (tempSQI > 100)? 100 : (uint8_t) tempSQI;
    } 
    else {
        *pQuality = 100;
    }

    SONY_TRACE_RETURN (result);
}



/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
/* Tuner Configuration */
//#define TUNER_SONY_ASCOT2D         /**< Define for Sony Ascot2D tuner. */
#define TUNER_IFAGCPOS             /**< Define for IFAGC sense positive. */
//#define TUNER_SPECTRUM_INV         /**< Define for spectrum inversion. */
#define TUNER_RFLVLMON_DISABLE     /**< Define to disable RF level monitoring. */

int SONY_TUNER_OFFSET_CUTOFF_HZ = 100000;

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

//#define MONITOR_LOOP_COUNT  1   /* Number of times to run the channel monitors */

/*------------------------------------------------------------------------------
 Const char definitions
------------------------------------------------------------------------------*/
#if 1
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
#endif

//dvbc
/* Comment out definitions below to disable monitor output */
#define MONITOR_SYNCSTAT            
#define MONITOR_IFAGCOUT
#define MONITOR_QAM
#define MONITOR_SYMBOLRATE
#define MONITOR_CARRIEROFFSET
#define MONITOR_SPECTRUMSENSE
#define MONITOR_SNR
#define MONITOR_PRERSBER
#define MONITOR_PACKETERRORNUMBER
#define MONITOR_PER
#define MONITOR_QUALITY
#define MONITOR_SSI

//#define MONITOR_LOOP_COUNT  1   /* Number of times to run the channel monitors */
/*------------------------------------------------------------------------------
 Const char definitions
------------------------------------------------------------------------------*/
static const char *DVBC_Modulation[] = { "16QAM", "32QAM", "64QAM", "128QAM", "256QAM" };

//DVBT2
/* Comment out definitions below to disable monitor output */
#define MONITOR_SYNCSTAT            
#define MONITOR_CARRIEROFFSET
#define MONITOR_IFAGCOUT
#define MONITOR_L1PRE
#define MONITOR_VERSION
#define MONITOR_OFDM
#define MONITOR_DATAPLPS
#define MONITOR_ACTIVEPLP
#define MONITOR_DATAPLPERROR
#define MONITOR_L1CHANGE
#define MONITOR_L1POST
#define MONITOR_SPECTRUMSENSE
#define MONITOR_SNR
#define MONITOR_PRELDPCBER
#define MONITOR_POSTBCHFER
#define MONITOR_PREBCHBER
#define MONITOR_PACKETERRORNUMBER
#define MONITOR_PPMOFFSET
#define MONITOR_TSRATE
#define MONITOR_QUALITY
#define MONITOR_PER

//#define MONITOR_LOOP_COUNT  1   /* Number of times to run the channel monitors */
/*------------------------------------------------------------------------------
 Const char definitions
------------------------------------------------------------------------------*/
#if 1
static const char *DVBT2_TuneInfo[] = { "OK", "Invalid PLP ID", "Invalid T2 Mode"};
static const char *DVBT2_Version[] = { "1.1.1", "1.2.1", "1.3.1" };
static const char *DVBT2_S1Signalling[] = { "SISO", "MISO", "Non-DVBT2" };
static const char *DVBT2_GaurdInterval[] = { "1/32", "1/16", "1/8", "1/4", "1/128", "19/128", "19/256" };
static const char *DVBT2_FFTMode[] = { "2k", "8k", "4k", "1k", "16k", "32k" };
static const char *DVBT2_L1PreType[] = { "TS", "GS", "TS & GS" };
static const char *DVBT2_PAPR[] = { "None", "ACE", "TR", "TR & ACE" };
static const char *DVBT2_L1PostModulation[] = { "BPSK", "QPSK", "16QAM", "64QAM" };
static const char *DVBT2_L1PostCodeRate[] = { "1/2" };
static const char *DVBT2_L1PostFEC[] = { "16k" };
static const char *DVBT2_PPMode[] = { "PP1", "PP2", "PP3", "PP4", "PP5", "PP6", "PP7", "PP8" };
static const char *DVBT2_CodeRate[] = { "1/2", "3/5", "2/3", "3/4", "4/5", "5/6" };
static const char *DVBT2_PLPModulation[] = { "QPSK", "16QAM", "64QAM", "256QAM" };
static const char *DVBT2_PLPType[] = { "Common", "Data Type 1", "Data Type 2" };
static const char *DVBT2_PLPPayload[] = { "GFPS", "GCS", "GSE", "TS" };
static const char *DVBT2_FEC[] = { "16k", "64k" };
#endif

sony_integ_t integ;
sony_demod_t demod;
sony_tuner_terr_cable_t tunerTerrCable;


sony_i2c_t i2c;
sony_i2c_t tunerI2C;
sony_dvbt_tune_param_t tuneParam;
sony_dvbt2_tune_param_t tuneParam_t2;
sony_dvbc_tune_param_t tuneParam_c;
sony_dvbc2_tune_param_t tuneParam_c2;
//sony_dvbc2_tune_params_t c2Params;
sony_ascot2e_t ascot2e;
sony_ascot3_t ascot3;



extern unsigned char TunMode;

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
int sony_cxd2837_init(void)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_result_t tuneResult = SONY_RESULT_OK;
    

	printf("[sony_cxd2837_init]-->1,result=%d\n",result);

    uint8_t i = 0;

    /*------------------------------------------------------------------------------
     Setup / Initialisation
    ------------------------------------------------------------------------------*/
    /* Create I2C interface for tuner and demosulator parts.  GW (GateWay) members 
       provided but not required for tuner communication.  I2C switching handled 
       internally in the driver using a repeater. */
    i2c.gwAddress = cxd2837adr;                                   /* N/A */
    i2c.gwSub = 0x09;                                       /* N/A */
    i2c.Read = Cxd2837_I2c_Read;                           /* Base level HW interfacing I2C read function */
    i2c.Write = Cxd2837_I2c_Write;                         /* Base level HW interfacing I2C write function */
    i2c.ReadRegister = sony2837_i2c_CommonReadRegister;         /* Common wrapper function for multi byte Read operation */
    i2c.WriteRegister = sony2837_i2c_CommonWriteRegister;       /* Common wrapper function for multi byte Write operation */
    i2c.WriteOneRegister = sony2837_i2c_CommonWriteOneRegister; /* Common wrapper function for single byte Write operation */

	    /* Setup I2C interfaces. */
    tunerI2C.gwAddress = cxd2837adr;
    tunerI2C.gwSub = 0x09;      /* Connected via demod I2C gateway function. */
   // tunerI2C.Read = Cxd2837_I2c_ReadGw;
	  tunerI2C.Read = Cxd2837_I2c_Read;
    tunerI2C.Write = Cxd2837_I2c_WriteGw;
    tunerI2C.ReadRegister = sony2837_i2c_CommonReadRegister;
    tunerI2C.WriteRegister = sony2837_i2c_CommonWriteRegister;
    tunerI2C.WriteOneRegister = sony2837_i2c_CommonWriteOneRegister;
	printf("[sony_cxd2837_init]-->2,result=%d\n",result);

if((TunMode==37)/*||(TunMode==45)*/)
{
	printf("[sony_cxd2837_init]-->3,result=%d\n",result);
	uint8_t xtalFreqMHz =41;
	uint8_t i2cAddress = SONY_ASCOT2E_ADDRESS;
	uint32_t configFlags = SONY_ASCOT2E_CONFIG_EXT_REF;
	
	if(TunMode==45){
	xtalFreqMHz = SONY_ASCOT3_XTAL_41000KHz;
	configFlags |= SONY_ASCOT3_CONFIG_LOOPTHRU_ENABLE;
	result |= sony_tuner_ascot3_Create (&tunerTerrCable, xtalFreqMHz, i2cAddress, &i2c, configFlags, &ascot3);
		}
	else{
	result |= sony_tuner_ascot2e_Create (&tunerTerrCable, xtalFreqMHz, i2cAddress, &i2c, configFlags, &ascot2e);	
		}
	if (result == SONY_RESULT_OK) {
		printf (" Tuner Created with the following parameters:\n");
		printf ("  - Tuner Type 	: CXD2831 (ASCOT2E) \n");
		printf ("  - XTal Frequency : %uMHz\n", xtalFreqMHz);
		printf ("  - I2C Address	: %u\n", i2cAddress);
		printf ("  - Config Flags	: %u\n\n", configFlags);
	}
	else {
		printf (" Error: Unable to create Sony ASCOT2E tuner driver. (result = %s)\n", Common_Result[result]);
		return -1;
	}
	printf("[sony_cxd2837_init]-->4,result=%d\n",result);

}

else
{printf("[sony_cxd2837_init]-->5,result=%d\n",result);
    {
        uint8_t xtalFreqMHz = 4;
        uint8_t i2cAddress = 0xc2;
        uint32_t configFlags = 0;

	result |=  cxd2837_mopll_tuner_Create (i2cAddress,xtalFreqMHz,&tunerI2C, 0, NULL, &tunerTerrCable);
	}printf("[sony_cxd2837_init]-->6,result=%d\n",result);
}

    /* Create the integration structure which contains the demodulaor and tuner part instances.  This 
     * function also internally Creates the demodulator part.  Once created the driver is in 
     * SONY_DEMOD_STATE_INVALID and must be initialized before calling a Tune / Scan or Monitor API. 
     */
    {
        /* Modify the following to suit your implementation */

        sony_demod_xtal_t xtalFreq = SONY_DEMOD_XTAL_16000KHz;
		int8_t i2cAddress = cxd2837adr;
		if(drxk_drxj == 5)
		{
			xtalFreq = SONY_DEMOD_XTAL_41000KHz;
		}

		if((TunMode==37)||(TunMode==35)||(TunMode==36)||(TunMode==44)||(TunMode==45)||(TunMode==42)||(TunMode==27))
		{
			 xtalFreq = SONY_DEMOD_XTAL_41000KHz;
		}

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
        result |= sony_integ_Create (&integ, xtalFreq, i2cAddress, &i2c, &demod
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
    result |= sony_integ_InitializeT_C (&integ);
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

	
switch(TunMode){
	case 41:
			SONY_TUNER_OFFSET_CUTOFF_HZ = 50000;
		    /* DVB-T demodulator IF configuration for ASCOT2D Low IF tuner */
		    demod.iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
		    demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
		    demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.50);
		    demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);

		    /* DVB-T2 demodulator IF configuration for ASCOT2D Low IF tuner */
		    demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
		    demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
		    demod.iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.50);
		    demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);

		    /* DVB-C demodulator IF configuration for ASCOT2D Low IF tuner */
		    demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);

		    /* DVB-C2 demodulator IF configuration for ASCOT2D Low IF tuner */
		    demod.iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		    demod.iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		break;
	case 36:
		SONY_TUNER_OFFSET_CUTOFF_HZ = 50000;
		    /* DVB-T demodulator IF configuration for ASCOT2D Low IF tuner */
		    demod.iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
		    demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
		    demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.50);
		    demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);

		    /* DVB-T2 demodulator IF configuration for ASCOT2D Low IF tuner */
		    demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
		    demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
		    demod.iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.50);
		    demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);

		    /* DVB-C demodulator IF configuration for ASCOT2D Low IF tuner */
		    demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);

		    /* DVB-C2 demodulator IF configuration for ASCOT2D Low IF tuner */
		    demod.iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		    demod.iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		break;
	case 37:
		SONY_TUNER_OFFSET_CUTOFF_HZ = 50000;
		    /* DVB-T demodulator IF configuration for ASCOT2D Low IF tuner */
		    demod.iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
		    demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
		    demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.20);
		    demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.80);

		    /* DVB-T2 demodulator IF configuration for ASCOT2D Low IF tuner */
		    demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
		    demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
		    demod.iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.20);
		    demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.80);

		    /* DVB-C demodulator IF configuration for ASCOT2D Low IF tuner */
		    demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.90);

		    /* DVB-C2 demodulator IF configuration for ASCOT2D Low IF tuner */
		    demod.iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.70);
		    demod.iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.90);
		break;
	case 44:
			SONY_TUNER_OFFSET_CUTOFF_HZ = 50000;
				/* DVB-T demodulator IF configuration for ORC53XX Low IF tuner */
			demod.iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
			demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
			demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
			demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		
			/* DVB-T2 demodulator IF configuration for ORC53XX Low IF tuner */
			demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
			demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
			demod.iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
			demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		
			/* DVB-C demodulator IF configuration for ORC53XX Low IF tuner */
			demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		break;
	case 45:
			SONY_TUNER_OFFSET_CUTOFF_HZ = 50000;
			/* DVB-T demodulator IF configuration for CXD2872 Low IF tuner */
			demod.iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
			demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
			demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.20);
			demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.80);
		
			/* DVB-T2 demodulator IF configuration for CXD2872 Low IF tuner */
			demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
			demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.60);
			demod.iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.20);
			demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.80);
		
			/* DVB-C demodulator IF configuration for CXD2872 Low IF tuner */
			demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.90);
		break;
	case 4:
			SONY_TUNER_OFFSET_CUTOFF_HZ = 166700;
			/* Configure demod driver for Sony ASCOT2D tuner IF. */
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
	break;
	case 12:
			SONY_TUNER_OFFSET_CUTOFF_HZ = 50000;
			/* Configure demod driver for Sony ASCOT2D tuner IF. */
			demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.5);
			demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
			demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
			demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);
			/* C2 ITB setup. */
			demod.iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);
			demod.iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);

			/* T2 ITB setup. */
			demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.0);
			demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.5);
			demod.iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
			demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
	break;
	case 29:
			SONY_TUNER_OFFSET_CUTOFF_HZ = 50000;
			/* Configure demod driver for Sony ASCOT2D tuner IF. */
			demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);
			demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);
			demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);
			demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);
			/* C2 ITB setup. */
			demod.iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);
			demod.iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);

			/* T2 ITB setup. */
			demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);
			demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);
			demod.iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);
			demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);
	break;
	case 26:
			SONY_TUNER_OFFSET_CUTOFF_HZ = 50000;
			/* Configure demod driver for Sony ASCOT2D tuner IF. */
			demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.07);
			demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.07);
			demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.07);
			demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.07);
			/* C2 ITB setup. */
			demod.iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.07);
			demod.iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.07);

			/* T2 ITB setup. */
			demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.07);
			demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.07);
			demod.iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.07);
			demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.07);
	break;
	case 20:
			SONY_TUNER_OFFSET_CUTOFF_HZ = 50000;
			/* Configure demod driver for Sony ASCOT2D tuner IF. */
			demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.25);
			demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.5);
			demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
			demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);

			/* C2 ITB setup. */
			demod.iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.6);
			demod.iffreqConfig.configDVBC2_8= SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);

			/* T2 ITB setup. */ 
			demod.iffreqConfig.configDVBT2_1_7 =  SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.0);
			demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.25);
			demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.25);
			demod.iffreqConfig.configDVBT2_7= SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.5);
			demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);


	break;
	case 21:
		SONY_TUNER_OFFSET_CUTOFF_HZ = 50000;
			/* Configure demod driver for Sony ASCOT2D tuner IF. */
		demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.25);
		demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
		demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.8);
		demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);

		/* C2 ITB setup. */
		demod.iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);
		demod.iffreqConfig.configDVBC2_8= SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.5);

		/* T2 ITB setup. */ 
		 demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.5);
		 demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.5);
		 demod.iffreqConfig.configDVBT2_7= SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.0);
		 demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.8);

	break;
	case 31:
	case 34:	
		SONY_TUNER_OFFSET_CUTOFF_HZ = 50000;
			/* DVB-T demodulator IF configuration for MXL603 Low IF tuner */
		demod.iffreqConfig.configDVBT_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.65);
		demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.65);
		demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.10);
		demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.57);

		/* DVB-T2 demodulator IF configuration for MXL603 Low IF tuner */
	//	demod.iffreqConfig.configDVBT2_1_7 =  SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.75);
		demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.65);
		demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.65);
		demod.iffreqConfig.configDVBT2_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.10);
		demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.57);

		/* DVB-C demodulator IF configuration for MXL603 Low IF tuner */
		demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);

		/* DVB-C2 demodulator IF configuration for MXL603 Low IF tuner */
		demod.iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (3.65);
		demod.iffreqConfig.configDVBC2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);

	break;
	case 32:
	case 35:
	case 28:
	case 38:
	case 42: //TDA18260	
	case 46:
		SONY_TUNER_OFFSET_CUTOFF_HZ = 50000;
			/* Configure demod driver for Sony ASCOT2D tuner IF. */
		demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);

		/* C2 ITB setup. */
		demod.iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		demod.iffreqConfig.configDVBC2_8= SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);

		/* T2 ITB setup. */ 
		 demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		 demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		 demod.iffreqConfig.configDVBT2_7= SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		 demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
	break;	 
	case 27:
		SONY_TUNER_OFFSET_CUTOFF_HZ = 50000;
			/* Configure demod driver for Sony ASCOT2D tuner IF. */
		demod.iffreqConfig.configDVBT_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		demod.iffreqConfig.configDVBT_7 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		demod.iffreqConfig.configDVBT_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		demod.iffreqConfig.configDVBC = SONY_DEMOD_MAKE_IFFREQ_CONFIG (4.75);

		/* C2 ITB setup. */
		demod.iffreqConfig.configDVBC2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		demod.iffreqConfig.configDVBC2_8= SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);

		/* T2 ITB setup. */ 
		 demod.iffreqConfig.configDVBT2_5 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		 demod.iffreqConfig.configDVBT2_6 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		 demod.iffreqConfig.configDVBT2_7= SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);
		 demod.iffreqConfig.configDVBT2_8 = SONY_DEMOD_MAKE_IFFREQ_CONFIG (5.0);

	break;
	
	default:
		return -1;
	break;
}


    /* IFAGC setup. Modify to suit connected tuner. */
    /* IFAGC positive, value = 0. */
//#ifdef TUNER_IFAGCPOS
	if((TunMode==37)||(TunMode==45))
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_IFAGCNEG, 1);
	else	
    	result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_IFAGCNEG, 0);
	
    if (result == SONY_RESULT_OK) {
        printf (" Demodulator configured for Negative IFAGC.\n");
    }
    else {
        printf (" Error: Unable to configure for Negative IFAGC. (result = %s)\n", Common_Result[result]);
        return -1;
    }
//#endif

	sony_demod_GPIOSetConfig(&demod,2,1,SONY_DEMOD_GPIO_MODE_TS_ERROR);

    /* Spectrum Inversion setup. Modify to suit connected tuner. */
    /* Spectrum inverted, value = 1. */
#ifdef TUNER_SPECTRUM_INV
    result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_SPECTRUM_INV, 1);
    if (result == SONY_RESULT_OK) {
        printf (" Demodulator configured for Inverted Spectrum.\n");
    }
    else {
        printf (" Error: Unable to configure SPECTRUM_INV. (result = %s)\n", Common_Result[result]);
        return -1;
    }
#endif


    /* RFAIN ADC and monitor enable/disable. */
    /* Default is disabled. 1: Enable, 0: Disable. */
#ifdef RFAIN_ADC_ENABLE
    result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_RFAIN_ENABLE, 0);
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
if(TunMode==37)
    demod.tunerOptimize = SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2D;
else if(TunMode==45)
	demod.tunerOptimize = SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2D;

else
    demod.tunerOptimize = SONY_DEMOD_TUNER_OPTIMIZE_UNKNOWN;

    printf (" Demodulator optimised for %s tuner.\n\n", Common_TunerOptimize[demod.tunerOptimize]);

#if 1 
		//***************************************************
		// add 2015.5.21 xu sel
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_PARALLEL_SEL, 0);// sel
						
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_SER_DATA_ON_MSB, 0); //Pin data[0]
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_OUTPUT_SEL_MSB, 1);  //MSB First
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_TSVALID_ACTIVE_HI, 1);
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_TSSYNC_ACTIVE_HI, 1);
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_TSERR_ACTIVE_HI, 1);		
						
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_LATCH_ON_POSEDGE, 1);//0 falling
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_TSCLK_CONT, 1);
						
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_TSCLK_MASK, 0);
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_TSVALID_MASK, 1);
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_TSERR_MASK, 0);
						
						
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_TSCLK_CURRENT_10mA, 1);
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_TS_CURRENT_10mA, 1);
		//result = sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_TS_BACKWARDS_COMPATIBLE, 1);
		result |= sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_TERR_CABLE_TS_SERIAL_CLK_FREQ, 2);//clk_65.6MHz
		//result = sony_demod_SetConfig (&demod, SONY_DEMOD_CONFIG_IFAGC_ADC_FS, 0);
		result |= sony_demod_SetTsClockModeAndFreq (&demod, SONY_DTV_SYSTEM_DVBT);
		printf("[sony_cxd2837_init]-->7,result=%d\n",result);
						
		//********************************************************
			
#endif
			
#if 0				
		sony2837_i2c_CommonWriteOneRegister (demod.pI2c, 0xc8, 0x00, 0x00) ;//20
				
		sony2837_i2c_CommonWriteOneRegister (demod.pI2c, 0xc8, 0x71, 0xF0) ;//DD	
#endif


	
	return 0;
}


int sony_cxd2837_DVBT_tun(unsigned long int centerFreqKHz,unsigned int BW_MHz)
{
	sony_result_t result = SONY_RESULT_OK;
    sony_result_t tuneResult = SONY_RESULT_OK;
    /* ---------------------------------------------------------------------------------
     * Tune
     * ------------------------------------------------------------------------------ */
   // printf ("------------------------------------------\n");
  //  printf (" DVB-T Tune   \n");
   // printf ("------------------------------------------\n");

    /* Configure the DVBT tune parameters based on the channel requirements */
    tuneParam.bandwidth = BW_MHz;          /* Channel bandwidth */
    tuneParam.centerFreqKHz = centerFreqKHz;                   /* Channel centre frequency in KHz */
    tuneParam.profile = SONY_DVBT_PROFILE_HP;           /* Channel profile for hierachical modes.  For non-hierachical use HP */

   // printf (" Tune to DVB-T signal with the following parameters:\n");
   // printf ("  - Center Freq    : %uKHz\n", tuneParam.centerFreqKHz);
   // printf ("  - Bandwidth      : %s\n", Common_Bandwidth[tuneParam.bandwidth]);
   // printf ("  - Profile        : %s\n", DVBT_Profile[tuneParam.profile]);
//
    /* Perform DVBT Tune */
    tuneResult = sony_integ_dvbt_Tune (&integ, &tuneParam);
   // printf ("  - Result         : %s\n\n", Common_Result[tuneResult]);
    /* ---------------------------------------------------------------------------------
     * Carrier Offset Compensation
     * ------------------------------------------------------------------------------ */
    /* Measure the current carrier offset and retune to compensate for cases outside the demodulator
     * acquisition range. */
#if 0
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
#endif
    return 0;
}

int sony_cxd2837_DVBC_tun(unsigned long int centerFreqKHz) 
{
    sony_result_t result = SONY_RESULT_OK;
    sony_result_t tuneResult = SONY_RESULT_OK;
    /* ---------------------------------------------------------------------------------
     * Tune
     * ------------------------------------------------------------------------------ */
   // printf ("------------------------------------------\n");
  //  printf (" Tune   \n");
  //  printf ("------------------------------------------\n");

    /* Configure the DVBC tune parameters based on the channel requirements */
    tuneParam_c.centerFreqKHz = centerFreqKHz;    /* Channel center frequency in KHz */
        
   // printf (" Tune to DVB-C signal with the following parameters:\n");
  //  printf ("  - Center Frequency    : %uKHz\n", tuneParam_c.centerFreqKHz);


    /* Perform DVBC Tune */
    tuneResult = sony_integ_dvbc_Tune (&integ, &tuneParam_c);
  //  printf ("  - Result              : %s\n\n", Common_Result[tuneResult]);
#if 0
    /* ---------------------------------------------------------------------------------
     * Carrier Offset Compensation
     * ------------------------------------------------------------------------------ */
    /* Measure the current carrier offset and retune to compensate for cases outside the demodulator
     * acquisition range. */
    if ((tuneResult == SONY_RESULT_ERROR_TIMEOUT) || (tuneResult == SONY_RESULT_OK) || (tuneResult == SONY_RESULT_OK_CONFIRM)) {
        int32_t offsetHz = 0;
        uint32_t stepHz = SONY_TUNER_OFFSET_CUTOFF_HZ;

        /* Monitor carrier offset. */
        result = sony_demod_dvbc_monitor_CarrierOffset (integ.pDemod, &offsetHz);
        if (result != SONY_RESULT_OK) {
            printf ("Error: Unable to monitor DVBC carrier offset. (result = %s)\n", Common_Result[result]);
            return -1;
        }

        printf (" DVB-C carrier offset of %ldHz detected.\n", offsetHz);

        /* Carrier recovery loop locked (demod locked), compensate for the offset and retry tuning. */
        stepHz = (stepHz + 1) / 2;
        if ((uint32_t) abs (offsetHz) > stepHz) {
            /* Tuners have only a fixed frequency step size (stepkHz), therefore we must query the tuner driver to get the actual
             * center frequency set by the tuner. */
            tuneParam_c.centerFreqKHz = (uint32_t) ((int32_t) integ.pTunerTerrCable->frequencyKHz + ((offsetHz + 500) / 1000));

            printf (" Re-tuning to compensate offset. New parameters:\n");
            printf ("  - Center Freq    : %uKHz\n", tuneParam_c.centerFreqKHz);

            /* Perform DVBT2 Tune */
            tuneResult = sony_integ_dvbc_Tune (&integ, &tuneParam);
            printf ("  - Result         : %s\n", Common_Result[tuneResult]);
        }
        else {
            printf (" Carrier offset compensation not required.\n");
        }
    }
    if ((tuneResult != SONY_RESULT_OK) && (tuneResult != SONY_RESULT_OK_CONFIRM)) {
        printf (" Error: Unable to get TS lock DVB-C signal at %lukHz. (status=%d, %s)\n", tuneParam_c.centerFreqKHz, tuneResult, Common_Result[result]);
        return -1;
    }

    printf (" TS locked to DVB-C signal at %lukHz.\n\n", tuneParam_c.centerFreqKHz);
#endif
	return 0;

}
unsigned  char TunerTone=0;

int sony_cxd2837_DVBT2_tun(unsigned long int centerFreqKHz,unsigned int BW_kHz)
{
	sony_result_t result = SONY_RESULT_OK;
    sony_result_t tuneResult = SONY_RESULT_OK;


    /* ---------------------------------------------------------------------------------
     * Tune
     * ------------------------------------------------------------------------------ */
 //   printf ("------------------------------------------\n");
  //  printf (" Tune   \n");
  //  printf ("------------------------------------------\n");

    /* Configure the DVBT2 tune parameters based on the channel requirements */
	if(BW_kHz==1700)
	{
		tuneParam_t2.bandwidth = SONY_DEMOD_BW_1_7_MHZ;          /* Channel bandwidth */
	}
	else if(BW_kHz==7000){
		tuneParam_t2.bandwidth = SONY_DEMOD_BW_7_MHZ;          /* Channel bandwidth */
	}
	else if(BW_kHz==6000){
		tuneParam_t2.bandwidth = SONY_DEMOD_BW_6_MHZ;          /* Channel bandwidth */
	}
	else
	{
		tuneParam_t2.bandwidth = SONY_DEMOD_BW_8_MHZ;          /* Channel bandwidth */
	}
    
    tuneParam_t2.centerFreqKHz = centerFreqKHz;                   /* Channel center frequency in KHz */
    tuneParam_t2.dataPlpId = TunerTone;                            /* PLP ID where multiple PLP's are available */
    /* Additional tune information fed back from the driver.  This parameter should be checked
       if the result from the tune call is SONY_RESULT_OK_CONFIRM. */
    tuneParam_t2.tuneInfo = SONY_DEMOD_DVBT2_TUNE_INFO_OK; 

   // printf (" Tune to DVB-T2 signal with the following parameters:\n");
 //   printf ("  - Center Freq    : %uKHz\n", tuneParam_t2.centerFreqKHz);
 //   printf ("  - Bandwidth      : %s\n", Common_Bandwidth[tuneParam_t2.bandwidth]);
  //  printf ("  - PLP ID         : %u\n", tuneParam_t2.dataPlpId);

    /* Perform DVBT2 Tune */
    tuneResult = sony_integ_dvbt2_Tune (&integ, &tuneParam_t2);
   // printf ("  - Result         : %s\n", Common_Result[tuneResult]);
   // printf ("  - Tune Info      : %s\n\n", DVBT2_TuneInfo[tuneParam_t2.tuneInfo]);
#if 0
    /* ---------------------------------------------------------------------------------
     * Confirm PLP ID
     * ------------------------------------------------------------------------------ */
    /* If the tune result = SONY_RESULT_OK_CONFIRM it means that the demodulator has acquired
       a channel at the provided frequency and bandwidth but there was an issue with the PLP
       selection.  If no matching PLP is found the demodulator will provide TS lock to the
       first entry in the L1 table */
    if ((tuneResult == SONY_RESULT_OK_CONFIRM) && (tuneParam_t2.tuneInfo == SONY_DEMOD_DVBT2_TUNE_INFO_INVALID_PLP_ID)) {
        sony_dvbt2_plp_t activePlpInfo;

        printf (" PLP ID error in acquisition:\n");

        result = sony_demod_dvbt2_monitor_ActivePLP (integ.pDemod, SONY_DVBT2_PLP_DATA, & activePlpInfo);
        if (result != SONY_RESULT_OK) {
            printf (" Error: Unable to monitor T2 Active PLP. (result = %s)\n", Common_Result[result]);
            return -1;
        }

        printf ("  - PLP Requested : %u\n", tuneParam_t2.dataPlpId);
        printf ("  - PLP Acquired  : %u\n\n", activePlpInfo.id);
    }

    /* ---------------------------------------------------------------------------------
     * Carrier Offset Compensation
     * ------------------------------------------------------------------------------ */
    /* Measure the current carrier offset and retune to compensate for cases outside the demodulator
     * acquisition range. */
    if ((tuneResult == SONY_RESULT_ERROR_TIMEOUT) || (tuneResult == SONY_RESULT_OK) || (tuneResult == SONY_RESULT_OK_CONFIRM)) {
        int32_t offsetHz = 0;
        uint32_t stepHz = SONY_TUNER_OFFSET_CUTOFF_HZ;

        /* Monitor carrier offset. */
        result = sony_demod_dvbt2_monitor_CarrierOffset (integ.pDemod, &offsetHz);
        if (result != SONY_RESULT_OK) {
            printf ("Error: Unable to monitor T2 carrier offset. (result = %s)\n", Common_Result[result]);
            return -1;
        }

        printf (" DVB-T2 carrier offset of %ldHz detected.\n", offsetHz);

        /* Carrier recovery loop locked (demod locked), compensate for the offset and retry tuning. */
        stepHz = (stepHz + 1) / 2;
        if ((uint32_t) abs (offsetHz) > stepHz) {
            /* Tuners have only a fixed frequency step size (stepkHz), therefore we must query the tuner driver to get the actual
             * center frequency set by the tuner. */
            tuneParam_t2.centerFreqKHz = (uint32_t) ((int32_t) integ.pTunerTerrCable->frequencyKHz + ((offsetHz + 500) / 1000));

            printf (" Re-tuning to compensate offset. New parameters:\n");
            printf ("  - Center Freq    : %uKHz\n", tuneParam_t2.centerFreqKHz);
            printf ("  - Bandwidth      : %s\n", Common_Bandwidth[tuneParam_t2.bandwidth]);
            printf ("  - PLP ID         : %u\n", tuneParam_t2.dataPlpId);
            /* Perform DVBT2 Tune */
            tuneResult = sony_integ_dvbt2_Tune (&integ, &tuneParam_t2);
            printf ("  - Result         : %s\n", Common_Result[tuneResult]);
            printf ("  - Tune Info      : %s\n\n", DVBT2_TuneInfo[tuneParam_t2.tuneInfo]);
        }
        else {
            printf (" Carrier offset compensation not required.\n");
        }
    }
    if ((tuneResult != SONY_RESULT_OK) && (tuneResult != SONY_RESULT_OK_CONFIRM)) {
        printf (" Error: Unable to get TS lock DVB-T2 signal at %lukHz. (status=%d, %s)\n", tuneParam_t2.centerFreqKHz, tuneResult, Common_Result[result]);
        return -1;
    }

    printf (" TS locked to DVB-T2 signal at %lukHz.\n\n", tuneParam_t2.centerFreqKHz);
#endif
return 0;
}

#if 1

int sony_cxd2837_DVBC2_tun(unsigned long int centerFreqKHz,unsigned int BW_MHz)
{


sony_result_t result = SONY_RESULT_OK;
    sony_result_t tuneResult = SONY_RESULT_OK;
//printf("sony_cxd2837_DVBC2_tun:%d %d\n",centerFreqKHz,BW_MHz);

  /* Configure the DVBC2 tune parameters based on the channel requirements */
    tuneParam_c2.c2TuningFrequencyKHz = centerFreqKHz;    /* DVBC2 channel tuning frequency */
if(BW_MHz>6)
    tuneParam_c2.bandwidth = SONY_DEMOD_BW_8_MHZ;  /* Channel bandwidth */
else
    tuneParam_c2.bandwidth = SONY_DEMOD_BW_6_MHZ;  /* Channel bandwidth */

    tuneParam_c2.dataSliceId = 0;                  /* Data slice ID */
    tuneParam_c2.dataPLPId = 0;                    /* Data PLP ID */
    tuneParam_c2.isDependentStaticDS = 0;          /* Is the data slice dependant static type */
    tuneParam_c2.c2TuningFrequencyDSDSKHz = 0;     /* DVBC2 channel tuning frequency for DSDS case */

    /* The frequency that will be used to configure the tuner.  This can be used in combination 
       with a different frequency in c2TuningFrequencyKHz to compensate for IF offsets in the RF 
       part.  If set to 0 the c2TuningFrequencyKHz and c2TuningFrequencyDSDSKHz settings will be used
       for the RF part. */
    tuneParam_c2.rfTuningFrequencyKHz = 0;
    
  //  printf (" Tune to DVB-C2 signal with the following parameters:\n");
   // printf ("  - C2 Tuning Frequency        : %uKHz\n", tuneParam.c2TuningFrequencyKHz);
  //  printf ("  - Bandwidth                  : %s\n", Common_Bandwidth[tuneParam.bandwidth]);
  //  printf ("  - Data Slice ID              : %u\n", tuneParam.dataSliceId);
  //  printf ("  - Data PLP ID                : %u\n", tuneParam.dataPLPId);
   // printf ("  - Is Dependant Static DS     : %s\n", Common_YesNo[tuneParam.isDependentStaticDS]);
   // printf ("  - C2 Tuning Frequency DSDS   : %uKHz\n", tuneParam.c2TuningFrequencyDSDSKHz);
   // printf ("  - RF Tuning Frequency        : %uKHz\n", tuneParam.rfTuningFrequencyKHz);

    /* Perform DVBC2 Tune */
    tuneResult = sony_integ_dvbc2_Tune (&integ, &tuneParam_c2);
    //printf ("  - Result                     : %s\n\n", Common_Result[tuneResult]);
#if 0
    /* ---------------------------------------------------------------------------------
     * Carrier Offset Compensation
     * ------------------------------------------------------------------------------ */
    /* Measure the current carrier offset and retune to compensate for cases outside the demodulator
     * acquisition range. */
    if ((tuneResult == SONY_RESULT_ERROR_TIMEOUT) || (tuneResult == SONY_RESULT_OK) || (tuneResult == SONY_RESULT_OK_CONFIRM)) {
        int32_t offsetHz = 0;
        uint32_t stepHz = SONY_TUNER_ASCOT2D_OFFSET_CUTOFF_HZ;

        /* Monitor carrier offset. */
        result = sony_demod_dvbc2_monitor_CarrierOffset (integ.pDemod, &offsetHz);
        if (result != SONY_RESULT_OK) {
            printf ("Error: Unable to monitor C2 carrier offset. (result = %s)\n", Common_Result[result]);
            return -1;
        }

        printf (" DVB-C2 carrier offset of %ldHz detected.\n", offsetHz);

        /* Carrier recovery loop locked (demod locked), compensate for the offset and retry tuning. */
        stepHz = (stepHz + 1) / 2;
        if ((uint32_t) abs (offsetHz) > stepHz) {
            /* Tuners have only a fixed frequency step size (stepkHz), therefore we must query the tuner driver to get the actual
             * center frequency set by the tuner. 
             *
             * The carrier offset is applied to the RF Tuning Frequency.  The C2 system frequency will be correct already, so 
             * carrier offset is due to tuner variance.
             */
            tuneParam.rfTuningFrequencyKHz = (uint32_t) ((int32_t) integ.pTunerTerrCable->frequencyKHz + ((offsetHz + 500) / 1000));

            printf (" Re-tuning to compensate offset. New parameters:\n");
            printf ("  - C2 Tuning Frequency        : %uKHz\n", tuneParam.c2TuningFrequencyKHz);
            printf ("  - Bandwidth                  : %s\n", Common_Bandwidth[tuneParam.bandwidth]);
            printf ("  - Data Slice ID              : %u\n", tuneParam.dataSliceId);
            printf ("  - Data PLP ID                : %u\n", tuneParam.dataPLPId);
            printf ("  - Is Dependant Static DS     : %s\n", Common_YesNo[tuneParam.isDependentStaticDS]);
            printf ("  - C2 Tuning Frequency DSDS   : %uKHz\n", tuneParam.c2TuningFrequencyDSDSKHz);
            printf ("  - RF Tuning Frequency        : %uKHz\n", tuneParam.rfTuningFrequencyKHz);
            /* Perform DVBC2 Tune */
            tuneResult = sony_integ_dvbc2_Tune (&integ, &tuneParam);
            printf ("  - Result                     : %s\n\n", Common_Result[tuneResult]);
        }
        else {
            printf (" Carrier offset compensation not required.\n");
        }
    }
    if ((tuneResult != SONY_RESULT_OK) && (tuneResult != SONY_RESULT_OK_CONFIRM)) {
        printf (" Error: Unable to get TS lock DVB-C2 signal at %lukHz. (status=%d, %s)\n", tuneParam.c2TuningFrequencyKHz, tuneResult, Common_Result[result]);
        return -1;
    }
#endif
	return 0;
}
#endif

int sony_cxd2837_CheckID(void){
	int cxd2837chipId=0;
	unsigned char err;
	CXD2837_DemodType=0;
	LockXtal=0;
	//if(1)//(TunMode==31)
	//	{	
			unsigned char rReg[2]={0};
			unsigned char err1,err2;

			rReg[0]=0;
			err1 = I2cWrite8(0xc8,0,&rReg[0],1,1);
			err1 |= I2cRead8(0xc8,0xfd,&rReg[0],1);

			rReg[1]=0;
			err2 = I2cWrite8(0xd8,0,&rReg[1],1,1);
			err2 |= I2cRead8(0xd8,0xfd,&rReg[1],1);

		//	printf("err0:%d  err1: %d\n",err1,err2);
		//	printf("reg0:%x  reg1: %x\n",rReg[0],rReg[1]);

			if((err1)&&(err2))
			{

				rReg[0]=0;
				err1 = I2cWrite8(0xda,0,&rReg[0],1,1);
				err1 |= I2cRead8(0xda,0xfd,&rReg[0],1);
				if(err1==0)
					goto cxd2837_da;
				else
					return 0;
			}

			if((rReg[0]==0x00)&&(rReg[1]==0x00))
				return 0;	

		
			if(((rReg[0]==SONY_DEMOD_CHIP_ID_CXD2837)||(rReg[0]==SONY_DEMOD_CHIP_ID_CXD2843))&&\
				((rReg[1]==SONY_DEMOD_CHIP_ID_CXD2837)||(rReg[1]==SONY_DEMOD_CHIP_ID_CXD2843)))
				{
					CXD2837_DemodType=2;
					drxk_drxj = 1;
					return 1;
				}
			if((rReg[0]==SONY_DEMOD_CHIP_ID_CXD2837)||(rReg[0]==SONY_DEMOD_CHIP_ID_CXD2843))
			{
				CXD2837_DemodType=0;
				#if 1
				cxd2837adr = 0xc8;
				cxd2837_bInit=1;
				sony_cxd2837_init();
				#endif
				if(rReg[0]==SONY_DEMOD_CHIP_ID_CXD2837){
					drxk_drxj = 1;
					return 1;
				}
				else if(rReg[0]==SONY_DEMOD_CHIP_ID_CXD2839){
					drxk_drxj = 2;
					return 1;
				}
				else if(rReg[0]==SONY_DEMOD_CHIP_ID_CXD2841){
					drxk_drxj = 3;
					return 1;
				}
				else if(rReg[0]==SONY_DEMOD_CHIP_ID_CXD2842){
					drxk_drxj = 4;
					return 1;
				}
				else if(rReg[0]==SONY_DEMOD_CHIP_ID_CXD2843){
					drxk_drxj = 5;
					return 1;
				}
				else{
					drxk_drxj=0;
					return 0;
				}
				
				return 1;
			}
			 if((rReg[1]==SONY_DEMOD_CHIP_ID_CXD2837)||(rReg[1]==SONY_DEMOD_CHIP_ID_CXD2843))
			{
				CXD2837_DemodType=1;
				cxd2837adr = 0xd8;
				cxd2837_bInit=2;
				sony_cxd2837_init();
				if(rReg[1]==SONY_DEMOD_CHIP_ID_CXD2837){
					drxk_drxj = 1;
					return 1;
				}
				else if(rReg[1]==SONY_DEMOD_CHIP_ID_CXD2839){
					drxk_drxj = 2;
					return 1;
				}
				else if(rReg[1]==SONY_DEMOD_CHIP_ID_CXD2841){
					drxk_drxj = 3;
					return 1;
				}
				else if(rReg[1]==SONY_DEMOD_CHIP_ID_CXD2842){
					drxk_drxj = 4;
					return 1;
				}
				else if(rReg[1]==SONY_DEMOD_CHIP_ID_CXD2843){
					drxk_drxj = 5;
					return 1;
				}
				else{
					drxk_drxj=0;
					return 0;
				}
				
				return 1;
			}

			return 0;


cxd2837_da:
			if((rReg[0]==SONY_DEMOD_CHIP_ID_CXD2837)||(rReg[0]==SONY_DEMOD_CHIP_ID_CXD2843))
			{
				CXD2837_DemodType=0;
				cxd2837adr = 0xda;
				cxd2837_bInit=1;
				sony_cxd2837_init();
				if(rReg[0]==SONY_DEMOD_CHIP_ID_CXD2837){
					drxk_drxj = 1;
					return 1;
				}
				else if(rReg[0]==SONY_DEMOD_CHIP_ID_CXD2839){
					drxk_drxj = 2;
					return 1;
				}
				else if(rReg[0]==SONY_DEMOD_CHIP_ID_CXD2841){
					drxk_drxj = 3;
					return 1;
				}
				else if(rReg[0]==SONY_DEMOD_CHIP_ID_CXD2842){
					drxk_drxj = 4;
					return 1;
				}
				else if(rReg[0]==SONY_DEMOD_CHIP_ID_CXD2843){
					drxk_drxj = 5;
					return 1;
				}
				else{
					drxk_drxj=0;
					return 0;
				}
				
				return 1;
			}

	return 0;
						
	/*}
	else
		{

	cxd2837adr = 0xc8;
	err = I2cWrite8(cxd2837adr,0,&cxd2837chipId,1,1);
	if(err){
		cxd2837adr = 0xd8;
		err = I2cWrite8(cxd2837adr,0,&cxd2837chipId,1,1);
	}
	I2cRead8(cxd2837adr,0xfd,&cxd2837chipId,1);

	
	//printf("**********************chipid=%x\n",chipid);
	if(cxd2837chipId==SONY_DEMOD_CHIP_ID_CXD2837){
		drxk_drxj = 1;
		return 1;
	}
	else if(cxd2837chipId==SONY_DEMOD_CHIP_ID_CXD2839){
		drxk_drxj = 2;
		return 1;
	}
	else if(cxd2837chipId==SONY_DEMOD_CHIP_ID_CXD2841){
		drxk_drxj = 3;
		return 1;
	}
	else if(cxd2837chipId==SONY_DEMOD_CHIP_ID_CXD2842){
		drxk_drxj = 4;
		return 1;
	}
	else if(cxd2837chipId==SONY_DEMOD_CHIP_ID_CXD2843){
		drxk_drxj = 5;
		return 1;
	}
	else{
		drxk_drxj=0;
		return 0;
	}
		}

	return 0;
*/
}

int sony_cxd2837_CheckLock(unsigned char sys){
	sony_result_t result = SONY_RESULT_OK;
	if(sys==2){
		uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyUnlock = 0;
        result = sony_demod_dvbt2_monitor_SyncStat (integ.pDemod, &syncState, &tsLock, &earlyUnlock);
		if((tsLock)&&(result == SONY_RESULT_OK)){
			LockXtal=1;
			//printf("dvbt2_cxd2837Locked\n");
			return 1;}
	}

	else if(sys==3){
		uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyUnlock = 0;
        result = sony_demod_dvbc2_monitor_SyncStat (integ.pDemod, &syncState, &tsLock, &earlyUnlock);
		if((tsLock)&&(result == SONY_RESULT_OK)){
			LockXtal=1;
			//printf("dvbc2_cxd2837Locked\n");
			return 1;}
	}
	else if(sys==0)
	{
		uint8_t arLocked = 0;
        uint8_t tsLock = 0;
        uint8_t earlyUnlock = 0;
        result = sony_demod_dvbc_monitor_SyncStat (integ.pDemod, &arLocked, &tsLock, &earlyUnlock);
		if((tsLock)&&(result == SONY_RESULT_OK)){
			LockXtal=1;
			//printf("dvbc_cxd2837Locked\n");
			return 1;}
	}
	else
	{
		uint8_t syncState = 0;
        uint8_t tsLock = 0;
        uint8_t earlyUnlock = 0;
        result = sony_demod_dvbt_monitor_SyncStat (integ.pDemod, &syncState, &tsLock, &earlyUnlock);
		//printf("dvbt_tsLock=%d,dvbt_result=%d\n",tsLock,result);
		if((tsLock)&&(result == SONY_RESULT_OK)){
			LockXtal=1;
			//printf("dvbt_cxd2837Locked\n");
			return 1;}
	}

	//printf("cxd2837Unlocked\n");

	return 0;
}

int sony_cxd2837_GetPower(unsigned char sys)
{
	sony_result_t result = SONY_RESULT_OK;
	int ifAGCOut = 0;
	if(TunMode==45)
	{
		result = sony_demod_I2cRepeaterEnable (integ.pDemod, 0x01);
		if (result != SONY_RESULT_OK) 
		{
		    SONY_TRACE_RETURN (result);
		}
		ifAGCOut = cxd2872_GetRFlevel();
		printf("power=%d\n",ifAGCOut);
		result = sony_demod_I2cRepeaterEnable (integ.pDemod, 0x00);
		if (result != SONY_RESULT_OK)
		{
		    SONY_TRACE_RETURN (result);
		}	
		return ifAGCOut;

	}
	else
	{
		if(sys==1)
		{
			result = sony_demod_dvbt_monitor_IFAGCOut (integ.pDemod, &ifAGCOut);
		}
		else if(sys==2){  
			result = sony_demod_dvbt2_monitor_IFAGCOut (integ.pDemod, &ifAGCOut);
		}
		else if(sys==3){
			result = sony_demod_dvbc2_monitor_IFAGCOut (integ.pDemod, &ifAGCOut);
		}
		else{
	      		result = sony_demod_dvbc_monitor_IFAGCOut (integ.pDemod, &ifAGCOut);
		}
	}
	return ifAGCOut;
}

#if 0
int sony_cxd2837_GetPower(unsigned char sys){
sony_result_t result = SONY_RESULT_OK;
uint32_t ifAGCOut = 0;
{
	if(sys==1)
	{
        result = sony_demod_dvbt_monitor_IFAGCOut (integ.pDemod, &ifAGCOut);
	}
	else if(sys==2){  
        result = sony_demod_dvbt2_monitor_IFAGCOut (integ.pDemod, &ifAGCOut);
	}
	else if(sys==3){
        result = sony_demod_dvbc2_monitor_IFAGCOut (integ.pDemod, &ifAGCOut);
	}
	else{
      	result = sony_demod_dvbc_monitor_IFAGCOut (integ.pDemod, &ifAGCOut);
	}
}
	return ifAGCOut;
}
#endif

int sony_cxd2837_GetQuality(unsigned char sys){
sony_result_t result = SONY_RESULT_OK;
uint8_t quality;
	if(sys==1)
	{
        result = sony_demod_dvbt_monitor_Quality (integ.pDemod, &quality);
	}
	else if(sys==2){
        result = sony_demod_dvbt2_monitor_Quality (integ.pDemod, &quality);
	}
	else if(sys==3){
        result = sony_demod_dvbc2_monitor_Quality (integ.pDemod, &quality);
	}
	else{
        result = sony_demod_dvbc_monitor_Quality (integ.pDemod, &quality);
	}
	return quality;
}

int sony_cxd2837_GetErr(unsigned char sys)
{
sony_result_t result = SONY_RESULT_OK;
uint32_t ber;
	if(sys==1)
	{
        result = sony_demod_dvbt_monitor_PreViterbiBER (integ.pDemod, &ber);
	}
	else if(sys==2){
        result = sony_demod_dvbt2_monitor_PreLDPCBER (integ.pDemod, &ber);
	}
	else if(sys==3){
        result = sony_demod_dvbc2_monitor_PreLDPCBER (integ.pDemod, &ber);
	}
	else{		
        result = sony_demod_dvbc_monitor_PreRSBER (integ.pDemod, &ber);
	}
	return ber;
}
