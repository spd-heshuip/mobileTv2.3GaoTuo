/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-11-20 16:46:05 #$
  File Revision : $Revision:: 6393 $
------------------------------------------------------------------------------*/
/**
 @file    sony_demod_dvbc2.h

          This file provides the demodulator control interface specific to DVB-C2.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_DEMOD_DVBC2_H
#define SONY_DEMOD_DVBC2_H

#include "sony_common.h"
#include "sony_dvbc2.h"
#include "sony_demod.h"

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/
/** 
 @brief The duration in ms between sequential demod lock checks in the scan sequence
*/
#define SONY_DEMOD_DVBC2_WAIT_DEMOD_LOCK_INTERVAL   10

/** 
 @brief 2s timeout for demod lock in the scan sequence
*/
#define SONY_DEMOD_DVBC2_WAIT_DEMOD_LOCK            2000

/** 
 @brief The duration in ms between sequential L1 data ready checks in the scan sequence
*/
#define SONY_DEMOD_DVBC2_WAIT_L1_DATA_INTERVAL      10

/** 
 @brief 1s timeout for L1 data ready checks in the scan sequence
*/
#define SONY_DEMOD_DVBC2_WAIT_L1_DATA               1000

/** 
 @brief The duration in ms between sequential Preset Ready checks in the tune sequence
*/
#define SONY_DEMOD_DVBC2_WAIT_PRESET_READY_INTERVAL 10

/** 
 @brief 2s timeout for Preset Ready in the tune sequence
*/
#define SONY_DEMOD_DVBC2_WAIT_PRESET_READY          2000

/** 
 @brief Number of bytes for a single preset
*/
#define SONY_DEMOD_DVBC2_PS_SIZE                    23

/** 
 @brief Number of bytes of data for each individual notch
*/
#define SONY_DEMOD_DVBC2_NUM_REGS_PER_NOTCH         5

#define SONY_DEMOD_DVBC2_FILTER1                    0           /**< Co & Adjecsant channel Supression filter 1 */
#define SONY_DEMOD_DVBC2_FILTER2                    1           /**< Co & Adjecsant channel Supression filter 2 */
#define SONY_DEMOD_DVBC2_FILTER3                    2           /**< Co & Adjecsant channel Supression filter 3 */
#define SONY_DEMOD_DVBC2_FILTER4                    3           /**< Co & Adjecsant channel Supression filter 4 */

#define SONY_DEMOD_DVBC2_KHZ_FACTOR                 1000        /**< Factor to scale to KHz */

#define SONY_DVBC2_OFDM_CAR_MAX                     3408        /**< Maximum number of OFDM carriers */
#define SONY_DVBC2_OFDM_CAR_CENTER                  (SONY_DVBC2_OFDM_CAR_MAX/2)    /**< Center point of OFDM carriers */

/*------------------------------------------------------------------------------
 Enumerations
------------------------------------------------------------------------------*/
/** @brief DVB-C2 Tune Sequence states. */
typedef enum {
    SONY_DEMOD_DVBC2_SCAN_STAGE_1,            /**< Calls DemodSetting1 : Requests RF tune */
    SONY_DEMOD_DVBC2_SCAN_STAGE_2,            /**< Calls CheckDemodLock : Requests Sleep */
    SONY_DEMOD_DVBC2_SCAN_STAGE_3,            /**< Calls Change1stTrial : Requests N/A */
    SONY_DEMOD_DVBC2_SCAN_STAGE_4,            /**< Calls waitL1DataState : Requests Sleep and callback */
    SONY_DEMOD_DVBC2_SCAN_STAGE_5,            /**< Calls SetNextDS : Requests N/A */
    SONY_DEMOD_DVBC2_SCAN_STAGE_6,            /**< Calls waitL1DataState : Requests Sleep and callback */
    SONY_DEMOD_DVBC2_SCAN_STAGE_7,            /**< Calls CalculateSystemTuningFrequency : Requests N/A Does not return to Integration layer */
    SONY_DEMOD_DVBC2_SCAN_TUNE_NEXT,          /**< Calls SetPLPID if same DS : Requests  demod and RF tune if new DS */
    SONY_DEMOD_DVBC2_SCAN_FINALIZE            /**< Calls N/A : Requests N/A */
} sony_demod_dvbc2_scan_seq_state_t;


/** @brief DVB-C2 Tune Sequence states. */
typedef enum {
    SONY_DEMOD_DVBC2_TUNE_INITIALIZE = 0,     /**< Calls ReleasePreset : Requests Tune RF */
    SONY_DEMOD_DVBC2_TUNE_NON_DSDS_STAGE_1,   /**< Calls DemodSetting2 : Requests N/A */
    SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_1,       /**< Calls DemodSetting3 : Requests N/A */
    SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_2,       /**< Calls PresetReady1State : Requests Sleep */
    SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_3,       /**< Calls Change1stTrial, CalculateCenterFrequency, DemodSetting4 : Requests N/A */
    SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_4,       /**< Calls PresetReady2State : Requests Sleep */
    SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_5,       /**< Calls Change1stTrial, CalculatePresetValue : Requests Tune RF */
    SONY_DEMOD_DVBC2_TUNE_DSDS_STAGE_6,       /**< Calls DemodPreset : Requests N/A */
    SONY_DEMOD_DVBC2_TUNE_WAIT_LOCK,          /**< Calls CheckDemodLock [scan], CheckTSLock [tune] : Requests Sleep */
    SONY_DEMOD_DVBC2_TUNE_CHANGE_FIRST_TRIAL, /**< Calls Change1stTrial : Requests N/A */
    SONY_DEMOD_DVBC2_TUNE_FINALIZE            /**< Calls N/A : Requests N/A */
} sony_demod_dvbc2_tune_seq_state_t;         

/*------------------------------------------------------------------------------
 Structs
------------------------------------------------------------------------------*/
/** 
 @brief The tune parameters for a DVB-C2 signal
*/
typedef struct sony_dvbc2_tune_param_t {
    /**
     @brief For normal DS this is the absolute frequency of the specified DS.
            For Dependant Static DS this is the C2 system tuning frequency.
    */
    uint32_t c2TuningFrequencyKHz;

    /**
     @brief Bandwidth of the C2 channel
    */
    sony_demod_bandwidth_t bandwidth;
    
    /**
     @brief The Data Slice ID to select in acquisition.
    */
    uint8_t dataSliceId;

    /**
     @brief The PLP ID to select in acquisition.
    */
    uint8_t dataPLPId;

    /**
     @brief Boolean value indicating if dependent Data Slice
    */
    uint8_t isDependentStaticDS;

    /**
     @brief Absolute frequency of the specified dependant static DS.
    */
    uint32_t c2TuningFrequencyDSDSKHz; 

    /**
     @brief The frequency that will be used to configure the tuner.  This can
            be used in combination with a different frequency in c2TuningFrequencyKHz
            to compensate for IF offsets in the RF part.  If set to 0 the 
            c2TuningFrequencyKHz and c2TuningFrequencyDSDSKHz settings will be used
            for the RF part.
    */
    uint32_t rfTuningFrequencyKHz; 
} sony_dvbc2_tune_param_t;

/** 
 @brief DVBC2 notch position in carriers
*/
typedef struct sony_demod_dvbc2_notch_data_t {
    uint32_t notchStart;                /**< Notch start frequency (in carriers) relative to the whole spectrum */
    uint32_t notchEnd;                  /**< Notch end frequency (in carriers) relative to the whole spectrum */
} sony_demod_dvbc2_notch_data_t;

/** 
 @brief System tuning information for each given data slice and PLP
*/
typedef struct sony_demod_dvbc2_stf_data_t {
    uint8_t stfDsId;                    /**< Data slice ID */
    uint8_t stfPlpId;                   /**< PLP ID */
    uint32_t stfTuningPos;              /**< System tuning frequency */
} sony_demod_dvbc2_stf_data_t;

/** 
 @brief Data slice information
*/
typedef struct sony_demod_dvbc2_dataSlice_t {
    /**
     @brief The Data Slice ID to select in acquisition.
    */
    uint8_t dataSliceId;

    /**
     @brief The PLP IDs available to select in acquisition.
    */
    uint8_t dataPLPIds[SONY_DVBC2_PLP_MAX];

    /**
     @brief The number of PLPs in the current Data Slice.
    */
    uint8_t numPlp;

    /**
     @brief Value indicating if dependent Data Slice
    */
    uint8_t isDependentStaticDS;

    /**
     @brief Dataslice Tune Position
    */
    uint32_t dsTunePos;

    /**
     @brief Data for Dependant static Data Slice; tune to first 
            datalice information
    */
    sony_demod_dvbc2_stf_data_t stf;

    /**
     @brief Data Slice initial RF tune center frequency in kHz
    */
    uint32_t tuneFrequencyKHz;
} sony_demod_dvbc2_dataSlice_t;

/**
 @brief DVB-C2 Scan Sequence strcuture.
*/
typedef struct sony_demod_dvbc2_scan_seq_t {
    uint8_t running;                                    /**< Sequencer running / complete flag. */
    sony_demod_dvbc2_scan_seq_state_t state;            /**< Sequence state. Internal use only. */
    uint8_t tuneRequired;                               /**< RF tune required flag */
    uint8_t callbackRequired;                           /**< A callback is required */
    sony_result_t loopResult;                           /**< Result code for the last wait loop */
    sony_dvbc2_tune_param_t tuneParam;                  /**< Tune parameters for next found data slice */
    uint8_t dsTuneIndex;                                /**< Current data slice index in ds array */
    uint8_t plpTuneIndex;                               /**< Current PLP index in the ds[].dataPLPIDs array */

    /* Scan data storage */
    sony_demod_dvbc2_dataSlice_t ds[SONY_DVBC2_DS_MAX]; /**< DSLICE ID information */
    uint8_t numDs;                                      /**< Number of dataslices */
    uint8_t currentDsIndex;                             /**< Current data slice index in the ds array */
    sony_demod_dvbc2_notch_data_t notch[SONY_DVBC2_NUM_NOTCH_MAX];            /**< Notch data */
    uint8_t numNotch;                                   /**< Number of notches */
    sony_demod_dvbc2_stf_data_t stf;                    /**< System tuning frequency data */
    uint32_t sf;                                        /**< Start freqency */
    uint8_t gi;                                         /**< Guard Interval */
    uint8_t bnw;                                        /**< Notch width */
    uint16_t dx;                                        /**< Carrier spacing */
    uint32_t nextScanFrequency;                         /**< Next frequency for the scan to use instead of the specified step */
    uint32_t systemTuningFrequency;                     /**< System tuning frequency kHz */

    /* state machine time related storage */
    uint32_t sleepTime;                                 /**< Sleep duration requested from demod layer to integration */
    uint32_t currentTime;                               /**< Current time elapsed from sequence start */
    uint32_t loopStartTime;                             /**< Time that the current loop operation started */
} sony_demod_dvbc2_scan_seq_t;

/**
 @brief DVB-C2 Tune Sequence strcuture.
*/
typedef struct sony_demod_dvbc2_tune_seq_t {
    /* These members are used by the integration layer */
    uint8_t running;                                    /**< Sequencer running / complete flag. */
    uint8_t tuneRequired;                               /**< RF tune required flag */
    uint32_t rfFrequency;                               /**< Next RF frequency to be tuned to */
    sony_result_t lockResult;                           /**< Lock result from the wait TS lock function */
    uint32_t sleepTime;                                 /**< Sleep duration requested from demod layer to integration */
    uint32_t currentTime;                               /**< Current time elapsed from sequence start */
    uint32_t loopStartTime;                             /**< Time that the current loop operation started */

    /* Specified tune parameters */
    uint32_t tuneParamCenterFreq;                       /**< Center frequency parameter from tune params */
    sony_demod_bandwidth_t bandwidth;                   /**< Bandwidth of RF to tune to */
    sony_dvbc2_tune_param_t * pTuneData;                /**< C2 dependant tune parameters */

    /* Tuning Algorithm */
    sony_demod_dvbc2_tune_seq_state_t state;            /**< Sequence state. Internal use only. */
    uint8_t filterSelect;                               /**< Index of filter range to use */
    uint16_t casDagcTarget;                             /**<  */
    int16_t crcg2BnFreqOfst;                            /**<  */
    uint8_t trcgNominalRate[5];                         /**<  */
    uint8_t ps[SONY_DEMOD_DVBC2_PS_SIZE];               /**< Preset data */
    uint32_t centerFreq;                                /**< 2nd RF frequency to be tuned to for Dependant Static DS scenario */
    uint32_t tunePosDSDS;                               /**< C2 Tune Position DSDS represented in carriers */
} sony_demod_dvbc2_tune_seq_t;
/*-----------------------------------------------------------------------------
    Functions
-----------------------------------------------------------------------------*/

/**
 @brief Initialize the sony_demod_dvbc2_scan_seq_t structure 

 @param pDemod The demod instance.
 @param pScanSeq Internal structure used to store scan parameters and result data.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbc2_InitScanSeq (sony_demod_t * pDemod, 
                                            sony_demod_dvbc2_scan_seq_t * pScanSeq);

/**
 @brief Initialize the sony_demod_dvbc2_tune_seq_t structure 

 @param pDemod The demod instance.
 @param pTuneParams Tune parameters structure, populated by the calling application.
 @param pTuneSeq Internal structure used to store tune parameters and result data.

 @return    SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbc2_InitTuneSeq (sony_demod_t * pDemod, 
                                            sony_dvbc2_tune_param_t * pTuneParams,
                                            sony_demod_dvbc2_tune_seq_t * pTuneSeq);

/**
 @brief State machine based function used to step through the scan process for an
        individual C2 system.  This function is cranked by the ::sony_integ_dvbc2_Scan 
        function in sony_integ_dvbc2.c.

 @param pDemod The demod instance.
 @param pScanSeq Internal structure used to store scan parameters and result data.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbc2_ScanSeq (sony_demod_t * pDemod, 
                                        sony_demod_dvbc2_scan_seq_t * pScanSeq);

/**
 @brief State machine based function used to step through the tune process for a
        specified C2 channel.  This function is cranked by the ::sony_integ_dvbc2_Tune 
        function in sony_integ_dvbc2.c.

 @param pDemod The demod instance.
 @param pTuneSeq Internal structure used to store scan parameters and result data.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbc2_TuneSeq (sony_demod_t * pDemod, 
                                        sony_demod_dvbc2_tune_seq_t * pTuneSeq);

/**
 @brief Configures the demodulator for acquisition to DVB-C2 channels based on the
        parameters provided.  Called from ::sony_integ_dvbc2_Tune at various stages
        in the Tune sequence.

 @param pDemod Instance of demod control struct
 @param pTuneParam Channel tune parameters
 
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbc2_Tune (sony_demod_t * pDemod, 
                                     sony_dvbc2_tune_param_t * pTuneParam);

/**
 @brief Put the demodulator into sleep mode, called from ::sony_integ_SleepT_C.

 @param pDemod Instance of demod control struct
 
 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbc2_Sleep (sony_demod_t * pDemod);

/**
 @brief Check DVB-C2 demodulator lock status.

 @param pDemod Instance of demod control struct
 @param pLock Demod lock state

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbc2_CheckDemodLock (sony_demod_t * pDemod, 
                                               sony_demod_lock_result_t * pLock);

/**
 @brief Check DVB-C2 TS lock status.

 @param pDemod Instance of demod control struct
 @param pLock TS lock state

 @return SONY_RESULT_OK if successful.
*/
sony_result_t sony_demod_dvbc2_CheckTSLock (sony_demod_t * pDemod, 
                                            sony_demod_lock_result_t * pLock);

#endif /* SONY_DEMOD_DVBC2_H */
