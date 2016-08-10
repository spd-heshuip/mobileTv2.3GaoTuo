/*------------------------------------------------------------------------------

 <dev:header>
    Copyright(c) 2011 Sony Corporation.

    $Revision: 2675 $
    $Author: mrushton $

</dev:header>

------------------------------------------------------------------------------*/
/**
 @file    mopll_tuner.h

          This file provides an example MOPLL RF tuner driver.

*/
/*----------------------------------------------------------------------------*/

#ifndef MOPLL_TUNER_H_
#define MOPLL_TUNER_H_

/*------------------------------------------------------------------------------
 Includes
------------------------------------------------------------------------------*/
#include "sony_tuner_terr_cable.h"

/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/

#define SONY_DVB_TUNER_MOPLL_TUNER_ADDRESS          0xC2        /**< 8bit form */
#define SONY_DVB_TUNER_MOPLL_DVBT_REF_FREQ_HZ       166667      /**< 166.667kHz */  
#define SONY_DVB_TUNER_MOPLL_DVBC_REF_FREQ_HZ       62500       /**< 62.5kHz */
#define SONY_DVB_TUNER_MOPLL_IF_MHZ                 36.167        /**< 36.167MHz */



/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/

/**
 @brief Creates/sets up the MOPLL example tuner driver. 

    @note: Does not allocate any memory.
 
 @param i2cAddress The I2C address of the tuner, typically 
        ::SONY_DVB_TUNER_MOPLL_TUNER_ADDRESS.
 @param xtalFreq SONY_ARG_UNUSED. The crystal frequency in MHz (set to 4). 
 @param pI2c The I2C driver instance for the tuner driver to use.
 @param configFlags SONY_ARG_UNUSED.
 @param user User supplied data pointer.
 @param pTuner Pointer to struct to setup tuner driver.

 @param SONY_RESULT_OK if successful and tuner driver setup.
*/
sony_result_t cxd2837_mopll_tuner_Create (uint8_t i2cAddress,
                                      uint32_t xtalFreq,
                                      sony_i2c_t * pI2c, uint32_t configFlags, void *user, sony_tuner_terr_cable_t * pTuner);

/**
 @brief Initialize the tuner.

 @param pTuner Instance of the tuner driver.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t cxd2837_mopll_tuner_Initialize (sony_tuner_terr_cable_t * pTuner);

/**
 @brief Finalize the tuner.

 @param pTuner Instance of the tuner driver.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t cxd2837_mopll_tuner_Finalize (sony_tuner_terr_cable_t * pTuner);

/**
 @brief Perform RF tuning on tuner.
        After performing the tune, the application can read back 
        ::sony_tuner_terr_cable_t::frequency field to get the actual 
        tuner RF frequency. The actual tuner RF frequency is 
        dependent on the RF tuner step size.

 @param pTuner Instance of the tuner driver.
 @param frequency RF frequency to tune too (kHz)
 @param system The type of channel to tune too (DVB-T/C/T2).
 @param bandWidth The bandwidth of the channel in MHz.
        - DVB-C: 8MHz only.
        - DVB-T: 6, 7 or 8.
        - DVB-T2: 5, 6, 7 or 8.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t cxd2837_mopll_tuner_Tune (sony_tuner_terr_cable_t * pTuner, 
                                    uint32_t frequency, 
                                    sony_dtv_system_t system,
                                    sony_demod_bandwidth_t bandWidth);

/**
 @brief Sleep the tuner device (if supported).
 
 @param pTuner Instance of the tuner driver.

 @return SONY_RESULT_OK if successful.
*/
sony_result_t cxd2837_mopll_tuner_Sleep (sony_tuner_terr_cable_t * pTuner);

#endif /* MOPLL_TUNER_H_ */
