/*------------------------------------------------------------------------------
  Copyright 2012 Sony Corporation

  Last Updated  : $Date:: 2012-11-08 15:50:20 #$
  File Revision : $Revision:: 6371 $
------------------------------------------------------------------------------*/
#include "sony_integ.h"
#include "sony_integ_dvbc2.h"
#include "sony_demod.h"
#include "sony_demod_dvbc2_monitor.h"

#ifndef SONY_INTEG_DISABLE_ASCOT_TUNER
#include "sony_tuner_ascot2e.h"
#endif

extern unsigned char TunMode;


/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_integ_dvbc2_Tune(sony_integ_t * pInteg,
                                    sony_dvbc2_tune_param_t * pTuneParam)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_dvbc2_tune_seq_t tuneSeq;
    sony_stopwatch_t timer;
    uint32_t sleepStartTime = 0;
    uint32_t elapsedTime = 0;

    SONY_TRACE_ENTER ("sony_integ_dvbc2_Tune");

    if ((!pInteg) || (!pTuneParam) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
 
    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP_T_C) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C)) {
        /* This api is accepted in SLEEP_T_C and ACTIVE_T_C states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (!pInteg->pDemod->scanMode) {
        /* Clear cancellation flag. */
        sony_atomic_set (&(pInteg->cancel), 0);
    }

    /* Check bandwidth validity */
    if ((pTuneParam->bandwidth != SONY_DEMOD_BW_6_MHZ) && (pTuneParam->bandwidth != SONY_DEMOD_BW_8_MHZ)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    /* Confirm Tune parameters */
    if (pTuneParam->c2TuningFrequencyKHz == 0){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pTuneParam->isDependentStaticDS && (pTuneParam->c2TuningFrequencyDSDSKHz == 0)){
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Start stopwatch to measure total waiting time */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    result = sony_demod_dvbc2_InitTuneSeq (pInteg->pDemod, pTuneParam, &tuneSeq);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    while (tuneSeq.running) {
        if (tuneSeq.sleepTime == 0) {
            /* Set current total time before calling demod layer function. */
            /* This is timed from scan start */
            result = sony_stopwatch_elapsed(&timer, &tuneSeq.currentTime);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            result = sony_demod_dvbc2_TuneSeq (pInteg->pDemod, &tuneSeq);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            /* Check if RF tuning is invoked in demod layer. */
            if (tuneSeq.tuneRequired) {
                /* Enable I2C repeater for tuner comms */
                if ((pInteg->pTunerTerrCable) && (pInteg->pTunerTerrCable->Tune)) {
					//#ifndef SONY_DISABLE_I2C_REPEATER
					if((TunMode==37)||(TunMode==31)||(TunMode==34)||(TunMode==45))
					{

                    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }
						}
//#endif

                    result = pInteg->pTunerTerrCable->Tune (pInteg->pTunerTerrCable, tuneSeq.rfFrequency, SONY_DTV_SYSTEM_DVBC2, pTuneParam->bandwidth);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }

                    tuneSeq.tuneRequired = 0; /* FALSE */

//#ifndef SONY_DISABLE_I2C_REPEATER
if((TunMode==37)||(TunMode==31)||(TunMode==34)||(TunMode==45))

{

                    /* Disable I2C repeater for tuner comms */
                    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }
}
//#endif
                }
            }

            if (tuneSeq.sleepTime > 0) {
                /* Sleep requested, store start time now */
                result = sony_stopwatch_elapsed(&timer, &sleepStartTime);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }
            }
        }

        if (tuneSeq.sleepTime > 0) {
            /*    This polling interval is not always same as interval in scan sequence.
                This interval affects interval for checking cancellation flag. */
            result = sony_stopwatch_sleep(&timer, SONY_DVBC2_CHECK_CANCEL_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            result = sony_stopwatch_elapsed(&timer, &elapsedTime);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            if (elapsedTime - sleepStartTime >= tuneSeq.sleepTime) {
                /* Finish sleep */
                tuneSeq.sleepTime = 0;
            }
        }

        /* Sanity check on overall wait time. */
        result = sony_stopwatch_elapsed(&timer, &elapsedTime);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        if (elapsedTime > SONY_DVBC2_MAX_TUNE_DURATION) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
        }

        /* Check cancellation. */
        if (sony_atomic_read (&(pInteg->cancel)) != 0) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
        }
    }

    result = tuneSeq.lockResult;

    /* Deferred exit if tune failed */
    if (tuneSeq.lockResult != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (tuneSeq.lockResult);
    }

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_dvbc2_Scan(sony_integ_t * pInteg,
                                      sony_integ_dvbc2_scan_param_t * pScanParam,
                                      sony_integ_dvbc2_scan_callback_t callBack)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_integ_dvbc2_scan_result_t scanResult;
    sony_demod_dvbc2_scan_seq_t scanSeq;      
    sony_stopwatch_t timer;
    uint32_t sleepStartTime = 0;
    uint32_t elapsedTime = 0;
    uint32_t scanFreqkHz = pScanParam->startFrequencyKHz;

    SONY_TRACE_ENTER ("sony_integ_dvbc2_Scan");

    if ((!pInteg) || (!pScanParam) || (!callBack) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    
    if ((pInteg->pDemod->state != SONY_DEMOD_STATE_SLEEP_T_C) && (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C)) {
        /* This api is accepted in SLEEP_T_C and ACTIVE_T_C states only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Clear cancellation flag. */
    sony_atomic_set (&(pInteg->cancel), 0);

    /* Ensure the scan parameters are valid. */
    if (pScanParam->endFrequencyKHz < pScanParam->startFrequencyKHz) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }
    
    /* Check bandwidth validity */
    if ((pScanParam->bandwidth != SONY_DEMOD_BW_6_MHZ) && (pScanParam->bandwidth != SONY_DEMOD_BW_8_MHZ)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }
    
    /* Start stopwatch to measure total waiting time */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    /* Set scan mode enabled */
    sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x01);

    while (scanFreqkHz <= pScanParam->endFrequencyKHz) {
        scanResult.tuneResult = SONY_RESULT_OK;
        (void) sony_demod_dvbc2_InitScanSeq (pInteg->pDemod, &scanSeq);

        /* Set up the next scan frequency */
        if (pScanParam->bandwidth == SONY_DEMOD_BW_8_MHZ) {
            scanSeq.nextScanFrequency = scanFreqkHz + 2000;
        }
        else if (pScanParam->bandwidth == SONY_DEMOD_BW_6_MHZ) {
            scanSeq.nextScanFrequency = scanFreqkHz + 1500;
        }
        else {
            sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
            SONY_TRACE_RETURN(SONY_RESULT_ERROR_NOSUPPORT);
        }

        /* Set the tune center frequency to the current step for demod initialisation */
        scanSeq.tuneParam.c2TuningFrequencyKHz = scanFreqkHz;
        scanSeq.tuneParam.bandwidth = pScanParam->bandwidth;
        scanSeq.tuneParam.dataPLPId = 0;
        scanSeq.tuneParam.dataSliceId = 0;
        scanSeq.tuneParam.isDependentStaticDS = 0;
        scanSeq.tuneParam.rfTuningFrequencyKHz = 0;
        
        /* Enable acquisition on the demodulator. */
        result = sony_demod_dvbc2_Tune (pInteg->pDemod, &(scanSeq.tuneParam));
        if (result != SONY_RESULT_OK) {
            sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
            SONY_TRACE_RETURN (result);
        }

        if ((pInteg->pTunerTerrCable) && (pInteg->pTunerTerrCable->Tune)) {
			//#ifndef SONY_DISABLE_I2C_REPEATER
			if((TunMode==31)||(TunMode==34)||(TunMode==37)||(TunMode==45))
			{

            /* Enable I2C repeater for tuner comms */
            result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
            if (result != SONY_RESULT_OK) {
                sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
                SONY_TRACE_RETURN (result);
            }
				}
//#endif

            /* Perform RF tuning. */
            result = pInteg->pTunerTerrCable->Tune (pInteg->pTunerTerrCable, scanFreqkHz, SONY_DTV_SYSTEM_DVBC2, pScanParam->bandwidth);
            if (result != SONY_RESULT_OK) {
                sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
                SONY_TRACE_RETURN (result);
            }
    
	//#ifndef SONY_DISABLE_I2C_REPEATER
	if((TunMode==31)||(TunMode==34)||(TunMode==37)||(TunMode==45))
	{

            /* Disable I2C repeater for tuner comms */
            result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
            if (result != SONY_RESULT_OK) {
                sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
                SONY_TRACE_RETURN (result);
            }
		}
//#endif
        }

        /* Perform Soft reset to start acquisition */
        result = sony_demod_TuneEnd (pInteg->pDemod);
        if (result != SONY_RESULT_OK) {
            sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
            SONY_TRACE_RETURN (result);
        }

        while (scanSeq.running) {
            if (scanSeq.sleepTime == 0) {
                /* Set current total time before calling demod layer function. */
                /* This is timed from scan start */
                result = sony_stopwatch_elapsed(&timer, &scanSeq.currentTime);
                if (result != SONY_RESULT_OK) {
                    sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
                    SONY_TRACE_RETURN (result);
                }

                result = sony_demod_dvbc2_ScanSeq (pInteg->pDemod, &scanSeq);
                if (result != SONY_RESULT_OK) {
                    sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
                    SONY_TRACE_RETURN (result);
                }

                /* DVB-C2 Scan tunes to all potenial Dataslice/PLP combinations */
                if (scanSeq.tuneRequired) {
                    scanResult.tuneResult = sony_integ_dvbc2_Tune (pInteg, &scanSeq.tuneParam);

                    scanSeq.tuneRequired = 0; /* FALSE */
                        
                    switch (scanResult.tuneResult) {
                        /* Intentional fall through. */
                        case SONY_RESULT_OK:
                        case SONY_RESULT_ERROR_UNLOCK:
                        case SONY_RESULT_ERROR_TIMEOUT:
                            /* Do nothing */
                            break;

                        /* Intentional fall through. */
                        case SONY_RESULT_ERROR_ARG:
                        case SONY_RESULT_ERROR_I2C:
                        case SONY_RESULT_ERROR_SW_STATE:
                        case SONY_RESULT_ERROR_HW_STATE:
                        case SONY_RESULT_ERROR_RANGE:
                        case SONY_RESULT_ERROR_NOSUPPORT:
                        case SONY_RESULT_ERROR_CANCEL:
                        case SONY_RESULT_ERROR_OTHER:
                        default:
                        {
                            /* Serious error occurred -> cancel operation. */
                            sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
                            SONY_TRACE_RETURN (scanResult.tuneResult);
                        }
                    }
                }

                if (scanSeq.callbackRequired) 
                {
                    if (scanSeq.loopResult != SONY_RESULT_OK){
                        scanResult.tuneResult = scanSeq.loopResult;
                        scanSeq.loopResult = SONY_RESULT_OK;
                    }

                    if (scanResult.tuneResult == SONY_RESULT_OK || 
                        scanResult.tuneResult == SONY_RESULT_ERROR_UNLOCK ||
                        scanResult.tuneResult == SONY_RESULT_ERROR_TIMEOUT)
                    {
                        /* Set up callback structure to the latest data */                
                        scanResult.tuneParam = scanSeq.tuneParam;
                        scanResult.centerFreqKHz = scanSeq.tuneParam.c2TuningFrequencyKHz;    
                    }

                    if (callBack) {
                        callBack (pInteg, &scanResult, pScanParam);
                    }                    

                    scanSeq.callbackRequired = 0; /* FALSE */
                }    

                if (scanSeq.sleepTime > 0) {
                    /* Sleep requested, store start time now */
                    result = sony_stopwatch_elapsed(&timer, &sleepStartTime);
                    if (result != SONY_RESULT_OK) {
                        sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
                        SONY_TRACE_RETURN (result);
                    }
                }
            }

            if (scanSeq.sleepTime > 0) {
                /* This polling interval is not always same as interval in scan sequence.
                   This interval affects interval for checking cancellation flag. */
                result = sony_stopwatch_sleep(&timer, SONY_DVBC2_CHECK_CANCEL_INTERVAL);
                if (result != SONY_RESULT_OK) {
                    sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
                    SONY_TRACE_RETURN (result);
                }

                result = sony_stopwatch_elapsed(&timer, &elapsedTime);
                if (result != SONY_RESULT_OK) {
                    sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
                    SONY_TRACE_RETURN (result);
                }

                if (elapsedTime - sleepStartTime >= scanSeq.sleepTime) {
                    /* Finish sleep */
                    scanSeq.sleepTime = 0;
                }
            }

            /* Sanity check on overall wait time. */
            result = sony_stopwatch_elapsed(&timer, &elapsedTime);
            if (result != SONY_RESULT_OK) {
                sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
                SONY_TRACE_RETURN (result);
            }

            if (elapsedTime > SONY_DVBC2_MAX_SCAN_DURATION) {
                sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_TIMEOUT);
            }

            /* Check cancellation. */
            if (sony_atomic_read (&(pInteg->cancel)) != 0) {
                sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
            }
        }

        scanFreqkHz = scanSeq.nextScanFrequency;

        /* Setup next scan iteration frequency */
        scanSeq.tuneParam.c2TuningFrequencyKHz = scanFreqkHz;
    }

    /* Clear scan mode */
    result = sony_demod_terr_cable_SetScanMode(pInteg->pDemod, SONY_DTV_SYSTEM_DVBC2, 0x00);
    
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_dvbc2_WaitTSLock (sony_integ_t * pInteg)
{
    sony_result_t result = SONY_RESULT_OK;
    sony_demod_lock_result_t lock = SONY_DEMOD_LOCK_RESULT_NOTDETECT;
    sony_stopwatch_t timer;
    uint8_t continueWait = 1;
    uint32_t elapsed = 0;

    SONY_TRACE_ENTER ("sony_integ_dvbc2_WaitTSLock");

    if ((!pInteg) || (!pInteg->pDemod)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C) {
        /* This api is accepted in ACTIVE_T_C state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    /* Wait for TS lock */
    result = sony_stopwatch_start (&timer);
    if (result != SONY_RESULT_OK) {
        SONY_TRACE_RETURN (result);
    }

    for (;;) {
        result = sony_stopwatch_elapsed(&timer, &elapsed);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }
  
        if (elapsed >= SONY_DVBC2_WAIT_TS_LOCK) {
            continueWait = 0;
        }

        result = sony_demod_dvbc2_CheckTSLock (pInteg->pDemod, &lock);
        if (result != SONY_RESULT_OK) {
            SONY_TRACE_RETURN (result);
        }

        switch (lock) {
        case SONY_DEMOD_LOCK_RESULT_LOCKED:
            SONY_TRACE_RETURN (SONY_RESULT_OK);

        case SONY_DEMOD_LOCK_RESULT_UNLOCKED:
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_UNLOCK);

        default:
            /* continue waiting... */
            break;              
        }

        /* Check cancellation. */
        if (sony_atomic_read (&(pInteg->cancel)) != 0) {
            SONY_TRACE_RETURN (SONY_RESULT_ERROR_CANCEL);
        }

        if (continueWait) {
            result = sony_stopwatch_sleep (&timer, SONY_DVBC2_WAIT_LOCK_INTERVAL);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }
        } 
        else {
            result = SONY_RESULT_ERROR_TIMEOUT;
            break;
        }
    }
    
    SONY_TRACE_RETURN (result);
}

sony_result_t sony_integ_dvbc2_monitor_RFLevel (sony_integ_t * pInteg, int32_t * pRFLeveldB)
{
    sony_result_t result = SONY_RESULT_OK;

    SONY_TRACE_ENTER ("sony_integ_dvbc2_monitor_RFLevel");

    if ((!pInteg) || (!pInteg->pDemod) || (!pRFLeveldB)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    if (pInteg->pDemod->state != SONY_DEMOD_STATE_ACTIVE_T_C) {
        /* This api is accepted in ACTIVE state only */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    if (pInteg->pDemod->system != SONY_DTV_SYSTEM_DVBC2) {
        /* Not DVB-C2*/
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_SW_STATE);
    }

    switch (pInteg->pDemod->tunerOptimize) {
#ifndef SONY_INTEG_DISABLE_ASCOT_TUNER
    case SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2D:
        {
            uint32_t ifAgc;

            result = sony_demod_dvbc2_monitor_IFAGCOut(pInteg->pDemod, &ifAgc);
            if (result != SONY_RESULT_OK) {
                SONY_TRACE_RETURN (result);
            }

            /* Protect against overflow. IFAGC is unsigned 12-bit. */
            if (ifAgc > 0xFFF) {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
            }

            /* 2nd order polynomial relationship :
             * RF (dB) = -4E-6 * IFAGC^2 + 0.0594 * IFAGC - 131.24
             * RF (dB*1000) = ((-4 * IFAGC^2) + (59400* IFAGC) - 131240000) / 1000
             */
            *pRFLeveldB = (int32_t) ((-4 * (int32_t) (ifAgc * ifAgc)) + (59400 * (int32_t) ifAgc) - 131240000);
            *pRFLeveldB = (*pRFLeveldB < 0) ? *pRFLeveldB - 500 : *pRFLeveldB + 500;
            *pRFLeveldB /= 1000;
        }
        break;

    case SONY_DEMOD_TUNER_OPTIMIZE_ASCOT2E:
        {
            int32_t ifGain;
            int32_t rfGain;

            if (pInteg->pTunerTerrCable) {
           //#ifndef SONY_DISABLE_I2C_REPEATER
if(TunMode==37)
{
                    /* Enable the I2C repeater */
                    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x01);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }
}
           //     #endif

                result = sony_tuner_ascot2e_ReadGain (pInteg->pTunerTerrCable, &ifGain, &rfGain, 0);
                if (result != SONY_RESULT_OK) {
                    SONY_TRACE_RETURN (result);
                }

             //#ifndef SONY_DISABLE_I2C_REPEATER
if(TunMode==37)
{
                    /* Disable the I2C repeater */
                    result = sony_demod_I2cRepeaterEnable (pInteg->pDemod, 0x00);
                    if (result != SONY_RESULT_OK) {
                        SONY_TRACE_RETURN (result);
                    }
}
      //          #endif

                /* RF Level dBm = IFOUT - IFGAIN - RFGAIN
                 * IFOUT is the target IF level for tuner, -4.0dBm
                 */
                *pRFLeveldB = 10 * (-400 - ifGain - rfGain);

                /* Note : An implementation specific offset may be required
                 * to compensate for component gains / attenuations */
            }
            else {
                SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
            }
        }
        break;
#endif
    default:
        /* Please add RF level calculation for non ASCOT tuners. */
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_NOSUPPORT);
    }

    SONY_TRACE_RETURN (result);
}
