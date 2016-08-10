#ifndef __DEMOD_RTL2832_H
#define __DEMOD_RTL2832_H

/**

@file

@brief   RTL2832 demod module declaration

One can manipulate RTL2832 demod through RTL2832 module.
RTL2832 module is derived from DVB-T demod module.



@par Example:
@code

// The example is the same as the DVB-T demod example in dvbt_demod_base.h except the listed lines.



#include "demod_rtl2832.h"


...



int main(void)
{
	DVBT_DEMOD_MODULE *pDemod;

	DVBT_DEMOD_MODULE     DvbtDemodModuleMemory;
	BASE_INTERFACE_MODULE BaseInterfaceModuleMemory;
	I2C_BRIDGE_MODULE     I2cBridgeModuleMemory;


	...



	// Build RTL2832 demod module.
	BuildRtl2832Module(
		&pDemod,
		&DvbtDemodModuleMemory,
		&BaseInterfaceModuleMemory,
		&I2cBridgeModuleMemory,
		0x20,								// I2C device address is 0x20 in 8-bit format.
		CRYSTAL_FREQ_28800000HZ,			// Crystal frequency is 28.8 MHz.
		TS_INTERFACE_SERIAL,				// TS interface mode is serial.
		RTL2832_APPLICATION_STB,			// Application mode is STB.
		200,								// Update function reference period is 200 millisecond
		YES									// Function 1 enabling status is YES.
		);



	// See the example for other DVB-T demod functions in dvbt_demod_base.h

	...


	return 0;
}


@endcode

*/


//#include "dvbt_demod_base.h"




// Constants
#define INVALID_POINTER_VALUE		0
#define NO_USE						0

#define LEN_1_BYTE					1
#define LEN_2_BYTE					2
#define LEN_3_BYTE					3
#define LEN_4_BYTE					4
#define LEN_5_BYTE					5
#define LEN_6_BYTE					6
#define LEN_11_BYTE					11

#define LEN_1_BIT					1

#define BYTE_MASK					0xff
#define BYTE_SHIFT					8
#define HEX_DIGIT_MASK				0xf
#define BYTE_BIT_NUM				8
#define LONG_BIT_NUM				32

#define BIT_0_MASK					0x1
#define BIT_1_MASK					0x2
#define BIT_2_MASK					0x4
#define BIT_3_MASK					0x8

#define BIT_4_MASK					0x10
#define BIT_5_MASK					0x20
#define BIT_6_MASK					0x40
#define BIT_7_MASK					0x80


#define BIT_8_MASK					0x100
#define BIT_7_SHIFT					7
#define BIT_8_SHIFT					8



// I2C buffer length
// Note: I2C_BUFFER_LEN must be greater than I2cReadingByteNumMax and I2cWritingByteNumMax in BASE_INTERFACE_MODULE.
#define I2C_BUFFER_LEN				128





/// On/off status
typedef enum ON_OFF_STATUS
{
	OFF,		///<   Off
	ON,			///<   On
}ON_OFF_STATUS;


/// Yes/no status
typedef enum YES_NO_STATUS
{
	NO,			///<   No
	YES,		///<   Yes
}YES_NO_STATUS;


/// Lock status
typedef enum LOCK_STATUS
{
	NOT_LOCKED,			///<   Not locked
	LOCKED,				///<   Locked
}LOCK_STATUS;


/// Loss status
typedef enum LOSS_STATUS
{
	NOT_LOST,			///<   Not lost
	LOST,				///<   Lost
}LOSS_STATUS;


/// Function return status
typedef enum FUNCTION_RETURN_STATUS
{
	FUNCTION_SUCCESS,			///<   Execute function successfully.
	FUNCTION_ERROR,				///<   Execute function unsuccessfully.
}FUNCTION_RETURN_STATUS;


/// Crystal frequency
typedef enum CRYSTAL_FREQ_HZ
{
	CRYSTAL_FREQ_4000000HZ  =  4000000,			///<   Crystal frequency =    4.0 MHz
	CRYSTAL_FREQ_16000000HZ = 16000000,			///<   Crystal frequency =   16.0 MHz
	CRYSTAL_FREQ_16384000HZ = 16384000,			///<   Crystal frequency = 16.384 MHz
	CRYSTAL_FREQ_16457143HZ = 16457143,			///<   Crystal frequency = 16.457 MHz
	CRYSTAL_FREQ_20000000HZ = 20000000,			///<   Crystal frequency =   20.0 MHz
	CRYSTAL_FREQ_20250000HZ = 20250000,			///<   Crystal frequency =  20.25 MHz
	CRYSTAL_FREQ_20480000HZ = 20480000,			///<   Crystal frequency =  20.48 MHz
	CRYSTAL_FREQ_24000000HZ = 24000000,			///<   Crystal frequency =   24.0 MHz
	CRYSTAL_FREQ_25000000HZ = 25000000,			///<   Crystal frequency =   25.0 MHz
	CRYSTAL_FREQ_25200000HZ = 25200000,			///<   Crystal frequency =   25.2 MHz
	CRYSTAL_FREQ_26000000HZ = 26000000,			///<   Crystal frequency =   26.0 MHz
	CRYSTAL_FREQ_26690000HZ = 26690000,			///<   Crystal frequency =  26.69 MHz
	CRYSTAL_FREQ_27000000HZ = 27000000,			///<   Crystal frequency =   27.0 MHz
	CRYSTAL_FREQ_28800000HZ = 28800000,			///<   Crystal frequency =   28.8 MHz
	CRYSTAL_FREQ_32000000HZ = 32000000,			///<   Crystal frequency =   32.0 MHz
	CRYSTAL_FREQ_36000000HZ = 36000000,			///<   Crystal frequency =   36.0 MHz
}CRYSTAL_FREQ_HZ;


/// IF frequency
typedef enum IF_FREQ_HZ
{
	IF_FREQ_0HZ        =        0,			///<   IF frequency =      0 MHz
	IF_FREQ_3250000HZ  =  3250000,			///<   IF frequency =   3.25 MHz
	IF_FREQ_3570000HZ  =  3570000,			///<   IF frequency =   3.57 MHz
	IF_FREQ_4000000HZ  =  4000000,			///<   IF frequency =    4.0 MHz
	IF_FREQ_4570000HZ  =  4570000,			///<   IF frequency =   4.57 MHz
	IF_FREQ_4571429HZ  =  4571429,			///<   IF frequency =  4.571 MHz
	IF_FREQ_5000000HZ  =  5000000,			///<   IF frequency =    5.0 MHz
	IF_FREQ_36000000HZ = 36000000,			///<   IF frequency =   36.0 MHz
	IF_FREQ_36125000HZ = 36125000,			///<   IF frequency = 36.125 MHz
	IF_FREQ_36150000HZ = 36150000,			///<   IF frequency =  36.15 MHz
	IF_FREQ_36166667HZ = 36166667,			///<   IF frequency = 36.167 MHz
	IF_FREQ_36170000HZ = 36170000,			///<   IF frequency =  36.17 MHz
	IF_FREQ_43750000HZ = 43750000,			///<   IF frequency =  43.75 MHz
	IF_FREQ_44000000HZ = 44000000,			///<   IF frequency =   44.0 MHz
}IF_FREQ_HZ;


/// Spectrum mode
typedef enum SPECTRUM_MODE
{
	SPECTRUM_NORMAL,			///<   Normal spectrum
	SPECTRUM_INVERSE,			///<   Inverse spectrum
}SPECTRUM_MODE;
#define SPECTRUM_MODE_NUM		2


/// TS interface mode
typedef enum TS_INTERFACE_MODE
{
	TS_INTERFACE_PARALLEL,			///<   Parallel TS interface
	TS_INTERFACE_SERIAL,			///<   Serial TS interface
}TS_INTERFACE_MODE;
#define TS_INTERFACE_MODE_NUM		2


/// Diversity mode
typedef enum DIVERSITY_PIP_MODE
{
	DIVERSITY_PIP_OFF,				///<   Diversity disable and PIP disable
	DIVERSITY_ON_MASTER,			///<   Diversity enable for Master Demod
	DIVERSITY_ON_SLAVE,				///<   Diversity enable for Slave Demod
	PIP_ON_MASTER,					///<   PIP enable for Master Demod
	PIP_ON_SLAVE,					///<   PIP enable for Slave Demod
}DIVERSITY_PIP_MODE;
#define DIVERSITY_PIP_MODE_NUM		5




// Definitions

// Page register address
#define DVBT_DEMOD_PAGE_REG_ADDR		0x00


// Bandwidth modes
#define DVBT_BANDWIDTH_NONE			-1

typedef enum DVBT_BANDWIDTH_MODE
{
	DVBT_BANDWIDTH_6MHZ,
	DVBT_BANDWIDTH_7MHZ,
	DVBT_BANDWIDTH_8MHZ,
}DVBT_BANDWIDTH_MODE;

#define DVBT_BANDWIDTH_MODE_NUM		3


// Constellation
typedef enum DVBT_CONSTELLATION_MODE
{
	DVBT_CONSTELLATION_QPSK,
	DVBT_CONSTELLATION_16QAM,
	DVBT_CONSTELLATION_64QAM,
}DVBT_CONSTELLATION_MODE;
#define DVBT_CONSTELLATION_NUM		3


// Hierarchy
typedef enum DVBT_HIERARCHY_MODE
{
	DVBT_HIERARCHY_NONE,
	DVBT_HIERARCHY_ALPHA_1,
	DVBT_HIERARCHY_ALPHA_2,
	DVBT_HIERARCHY_ALPHA_4,
}DVBT_HIERARCHY_MODE;
#define DVBT_HIERARCHY_NUM			4


// Code rate
typedef enum DVBT_CODE_RATE_MODE
{
	DVBT_CODE_RATE_1_OVER_2,
	DVBT_CODE_RATE_2_OVER_3,
	DVBT_CODE_RATE_3_OVER_4,
	DVBT_CODE_RATE_5_OVER_6,
	DVBT_CODE_RATE_7_OVER_8,
}DVBT_CODE_RATE_MODE;
#define DVBT_CODE_RATE_NUM			5


// Guard interval
typedef enum DVBT_GUARD_INTERVAL_MODE
{
	DVBT_GUARD_INTERVAL_1_OVER_32,
	DVBT_GUARD_INTERVAL_1_OVER_16,
	DVBT_GUARD_INTERVAL_1_OVER_8,
	DVBT_GUARD_INTERVAL_1_OVER_4,
}DVBT_GUARD_INTERVAL_MODE;
#define DVBT_GUARD_INTERVAL_NUM		4


// FFT mode
typedef enum DVBT_FFT_MODE_MODE
{
	DVBT_FFT_MODE_2K,
	DVBT_FFT_MODE_8K,
}DVBT_FFT_MODE_MODE;
#define DVBT_FFT_MODE_NUM			2





// Register entry definitions

// Register entry
typedef struct
{
	int IsAvailable;
	unsigned long PageNo;
	unsigned char RegStartAddr;
	unsigned char Msb;
	unsigned char Lsb;
}
DVBT_REG_ENTRY;



// Primary register entry
typedef struct
{
	int RegBitName;
	unsigned long PageNo;
	unsigned char RegStartAddr;
	unsigned char Msb;
	unsigned char Lsb;
}
DVBT_PRIMARY_REG_ENTRY;





// Register table dependence

// Demod register bit names
typedef enum DVBT_REG_BIT_NAME
{
	// Software reset register
	DVBT_SOFT_RST,

	// Tuner I2C forwording register
	DVBT_IIC_REPEAT,


	// Registers for initializing
	DVBT_TR_WAIT_MIN_8K,
	DVBT_RSD_BER_FAIL_VAL,
	DVBT_EN_BK_TRK,
	DVBT_REG_PI,

	DVBT_REG_PFREQ_1_0,				// For RTL2830 only
	DVBT_PD_DA8,					// For RTL2830 only
	DVBT_LOCK_TH,					// For RTL2830 only
	DVBT_BER_PASS_SCAL,				// For RTL2830 only
	DVBT_CE_FFSM_BYPASS,			// For RTL2830 only
	DVBT_ALPHAIIR_N,				// For RTL2830 only
	DVBT_ALPHAIIR_DIF,				// For RTL2830 only
	DVBT_EN_TRK_SPAN,				// For RTL2830 only
	DVBT_LOCK_TH_LEN,				// For RTL2830 only
	DVBT_CCI_THRE,					// For RTL2830 only
	DVBT_CCI_MON_SCAL,				// For RTL2830 only
	DVBT_CCI_M0,					// For RTL2830 only
	DVBT_CCI_M1,					// For RTL2830 only
	DVBT_CCI_M2,					// For RTL2830 only
	DVBT_CCI_M3,					// For RTL2830 only
	DVBT_SPEC_INIT_0,				// For RTL2830 only
	DVBT_SPEC_INIT_1,				// For RTL2830 only
	DVBT_SPEC_INIT_2,				// For RTL2830 only

	DVBT_AD_EN_REG,					// For RTL2832 only
	DVBT_AD_EN_REG1,				// For RTL2832 only
	DVBT_EN_BBIN,					// For RTL2832 only
	DVBT_MGD_THD0,					// For RTL2832 only
	DVBT_MGD_THD1,					// For RTL2832 only
	DVBT_MGD_THD2,					// For RTL2832 only
	DVBT_MGD_THD3,					// For RTL2832 only
	DVBT_MGD_THD4,					// For RTL2832 only
	DVBT_MGD_THD5,					// For RTL2832 only
	DVBT_MGD_THD6,					// For RTL2832 only
	DVBT_MGD_THD7,					// For RTL2832 only
	DVBT_EN_CACQ_NOTCH,				// For RTL2832 only
	DVBT_AD_AV_REF,					// For RTL2832 only
	DVBT_PIP_ON,					// For RTL2832 only
	DVBT_SCALE1_B92,				// For RTL2832 only
	DVBT_SCALE1_B93,				// For RTL2832 only
	DVBT_SCALE1_BA7,				// For RTL2832 only
	DVBT_SCALE1_BA9,				// For RTL2832 only
	DVBT_SCALE1_BAA,				// For RTL2832 only
	DVBT_SCALE1_BAB,				// For RTL2832 only
	DVBT_SCALE1_BAC,				// For RTL2832 only
	DVBT_SCALE1_BB0,				// For RTL2832 only
	DVBT_SCALE1_BB1,				// For RTL2832 only
	DVBT_KB_P1,						// For RTL2832 only
	DVBT_KB_P2,						// For RTL2832 only
	DVBT_KB_P3,						// For RTL2832 only
	DVBT_OPT_ADC_IQ,				// For RTL2832 only
	DVBT_AD_AVI,					// For RTL2832 only
	DVBT_AD_AVQ,					// For RTL2832 only
	DVBT_K1_CR_STEP12,				// For RTL2832 only

	// Registers for initializing according to mode
	DVBT_TRK_KS_P2,
	DVBT_TRK_KS_I2,
	DVBT_TR_THD_SET2,
	DVBT_TRK_KC_P2,
	DVBT_TRK_KC_I2,
	DVBT_CR_THD_SET2,

	// Registers for IF setting
	DVBT_PSET_IFFREQ,
	DVBT_SPEC_INV,


	// Registers for bandwidth programming
	DVBT_BW_INDEX,					// For RTL2830 only

	DVBT_RSAMP_RATIO,				// For RTL2832 only
	DVBT_CFREQ_OFF_RATIO,			// For RTL2832 only


	// FSM stage register
	DVBT_FSM_STAGE,

	// TPS content registers
	DVBT_RX_CONSTEL,
	DVBT_RX_HIER,
	DVBT_RX_C_RATE_LP,
	DVBT_RX_C_RATE_HP,
	DVBT_GI_IDX,
	DVBT_FFT_MODE_IDX,
	
	// Performance measurement registers
	DVBT_RSD_BER_EST,
	DVBT_CE_EST_EVM,

	// AGC registers
	DVBT_RF_AGC_VAL,
	DVBT_IF_AGC_VAL,
	DVBT_DAGC_VAL,

	// TR offset and CR offset registers
	DVBT_SFREQ_OFF,
	DVBT_CFREQ_OFF,


	// AGC relative registers
	DVBT_POLAR_RF_AGC,
	DVBT_POLAR_IF_AGC,
	DVBT_AAGC_HOLD,
	DVBT_EN_RF_AGC,
	DVBT_EN_IF_AGC,
	DVBT_IF_AGC_MIN,
	DVBT_IF_AGC_MAX,
	DVBT_RF_AGC_MIN,
	DVBT_RF_AGC_MAX,
	DVBT_IF_AGC_MAN,
	DVBT_IF_AGC_MAN_VAL,
	DVBT_RF_AGC_MAN,
	DVBT_RF_AGC_MAN_VAL,
	DVBT_DAGC_TRG_VAL,

	DVBT_AGC_TARG_VAL,				// For RTL2830 only
	DVBT_LOOP_GAIN_3_0,				// For RTL2830 only
	DVBT_LOOP_GAIN_4,				// For RTL2830 only
	DVBT_VTOP,						// For RTL2830 only
	DVBT_KRF,						// For RTL2830 only

	DVBT_AGC_TARG_VAL_0,			// For RTL2832 only
	DVBT_AGC_TARG_VAL_8_1,			// For RTL2832 only
	DVBT_AAGC_LOOP_GAIN,			// For RTL2832 only
	DVBT_LOOP_GAIN2_3_0,			// For RTL2832 only
	DVBT_LOOP_GAIN2_4,				// For RTL2832 only
	DVBT_LOOP_GAIN3,				// For RTL2832 only
	DVBT_VTOP1,						// For RTL2832 only
	DVBT_VTOP2,						// For RTL2832 only
	DVBT_VTOP3,						// For RTL2832 only
	DVBT_KRF1,						// For RTL2832 only
	DVBT_KRF2,						// For RTL2832 only
	DVBT_KRF3,						// For RTL2832 only
	DVBT_KRF4,						// For RTL2832 only
	DVBT_EN_GI_PGA,					// For RTL2832 only
	DVBT_THD_LOCK_UP,				// For RTL2832 only
	DVBT_THD_LOCK_DW,				// For RTL2832 only
	DVBT_THD_UP1,					// For RTL2832 only
	DVBT_THD_DW1,					// For RTL2832 only
	DVBT_INTER_CNT_LEN,				// For RTL2832 only
	DVBT_GI_PGA_STATE,				// For RTL2832 only
	DVBT_EN_AGC_PGA,				// For RTL2832 only


	// TS interface registers
	DVBT_CKOUTPAR,
	DVBT_CKOUT_PWR,
	DVBT_SYNC_DUR,
	DVBT_ERR_DUR,
	DVBT_SYNC_LVL,
	DVBT_ERR_LVL,
	DVBT_VAL_LVL,
	DVBT_SERIAL,
	DVBT_SER_LSB,
	DVBT_CDIV_PH0,
	DVBT_CDIV_PH1,

	DVBT_MPEG_IO_OPT_2_2,			// For RTL2832 only
	DVBT_MPEG_IO_OPT_1_0,			// For RTL2832 only
	DVBT_CKOUTPAR_PIP,				// For RTL2832 only
	DVBT_CKOUT_PWR_PIP,				// For RTL2832 only
	DVBT_SYNC_LVL_PIP,				// For RTL2832 only
	DVBT_ERR_LVL_PIP,				// For RTL2832 only
	DVBT_VAL_LVL_PIP,				// For RTL2832 only
	DVBT_CKOUTPAR_PID,				// For RTL2832 only
	DVBT_CKOUT_PWR_PID,				// For RTL2832 only
	DVBT_SYNC_LVL_PID,				// For RTL2832 only
	DVBT_ERR_LVL_PID,				// For RTL2832 only
	DVBT_VAL_LVL_PID,				// For RTL2832 only


	// FSM state-holding register
	DVBT_SM_PASS,

	// Registers for function 2 (for RTL2830 only)
	DVBT_UPDATE_REG_2,

	// Registers for function 3 (for RTL2830 only)
	DVBT_BTHD_P3,
	DVBT_BTHD_D3,

	// Registers for function 4 (for RTL2830 only)
	DVBT_FUNC4_REG0,
	DVBT_FUNC4_REG1,
	DVBT_FUNC4_REG2,
	DVBT_FUNC4_REG3,
	DVBT_FUNC4_REG4,
	DVBT_FUNC4_REG5,
	DVBT_FUNC4_REG6,
	DVBT_FUNC4_REG7,
	DVBT_FUNC4_REG8,
	DVBT_FUNC4_REG9,
	DVBT_FUNC4_REG10,

	// Registers for functin 5 (for RTL2830 only)
	DVBT_FUNC5_REG0,
	DVBT_FUNC5_REG1,
	DVBT_FUNC5_REG2,
	DVBT_FUNC5_REG3,
	DVBT_FUNC5_REG4,
	DVBT_FUNC5_REG5,
	DVBT_FUNC5_REG6,
	DVBT_FUNC5_REG7,
	DVBT_FUNC5_REG8,
	DVBT_FUNC5_REG9,
	DVBT_FUNC5_REG10,
	DVBT_FUNC5_REG11,
	DVBT_FUNC5_REG12,
	DVBT_FUNC5_REG13,
	DVBT_FUNC5_REG14,
	DVBT_FUNC5_REG15,
	DVBT_FUNC5_REG16,
	DVBT_FUNC5_REG17,
	DVBT_FUNC5_REG18,


	// AD7 registers (for RTL2832 only)
	DVBT_AD7_SETTING,
	DVBT_RSSI_R,

	// ACI detection registers (for RTL2832 only)
	DVBT_ACI_DET_IND,

	// Clock output registers (for RTL2832 only)
	DVBT_REG_MON,
	DVBT_REG_MONSEL,
	DVBT_REG_GPE,
	DVBT_REG_GPO,
	DVBT_REG_4MSEL,


	// Test registers for test only
	DVBT_TEST_REG_1,
	DVBT_TEST_REG_2,
	DVBT_TEST_REG_3,
	DVBT_TEST_REG_4,

	// Item terminator
	DVBT_REG_BIT_NAME_ITEM_TERMINATOR,
}DVBT_REG_BIT_NAME;



// Register table length definitions
#define DVBT_REG_TABLE_LEN_MAX			DVBT_REG_BIT_NAME_ITEM_TERMINATOR






// Definitions

// Initializing
#define RTL2832_INIT_TABLE_LEN						32
#define RTL2832_TS_INTERFACE_INIT_TABLE_LEN			5
#define RTL2832_APP_INIT_TABLE_LEN					5


// Bandwidth setting
#define RTL2832_H_LPF_X_PAGE		1
#define RTL2832_H_LPF_X_ADDR		0x1c
#define RTL2832_H_LPF_X_LEN			32
#define RTL2832_RATIO_PAGE			1
#define RTL2832_RATIO_ADDR			0x9d
#define RTL2832_RATIO_LEN			6


// Bandwidth setting
#define RTL2832_CFREQ_OFF_RATIO_BIT_NUM		20


// IF frequency setting
#define RTL2832_PSET_IFFREQ_BIT_NUM		22


// Signal quality
#define RTL2832_SQ_FRAC_BIT_NUM			5


// BER
#define RTL2832_BER_DEN_VALUE				1000000


// SNR
#define RTL2832_CE_EST_EVM_MAX_VALUE		65535
#define RTL2832_SNR_FRAC_BIT_NUM			10
#define RTL2832_SNR_DB_DEN					3402


// AGC
#define RTL2832_RF_AGC_REG_BIT_NUM		14
#define RTL2832_IF_AGC_REG_BIT_NUM		14


// TR offset and CR offset
#define RTL2832_SFREQ_OFF_BIT_NUM		14
#define RTL2832_CFREQ_OFF_BIT_NUM		18


// Register table length
#define RTL2832_REG_TABLE_LEN			127


// Function 1
#define RTL2832_FUNC1_WAIT_TIME_MS			500
#define RTL2832_FUNC1_GETTING_TIME_MS		200
#define RTL2832_FUNC1_GETTING_NUM_MIN		20



/// Demod application modes
typedef enum RTL2832_APPLICATION_MODE
{
	RTL2832_APPLICATION_DONGLE,
	RTL2832_APPLICATION_STB,
}RTL2832_APPLICATION_MODE;
#define RTL2832_APPLICATION_MODE_NUM		2


// Function 1
typedef enum RTL2832_FUNC1_CONFIG_MODE
{
	RTL2832_FUNC1_CONFIG_1,
	RTL2832_FUNC1_CONFIG_2,
	RTL2832_FUNC1_CONFIG_3,
}RTL2832_FUNC1_CONFIG_MODE;
#define RTL2832_FUNC1_CONFIG_MODE_NUM		3
#define RTL2832_FUNC1_CONFIG_NORMAL			-1


typedef enum RTL2832_FUNC1_STATE
{
	RTL2832_FUNC1_STATE_NORMAL,
	RTL2832_FUNC1_STATE_NORMAL_GET_BER,
	RTL2832_FUNC1_STATE_CONFIG_1_WAIT,
	RTL2832_FUNC1_STATE_CONFIG_1_GET_BER,
	RTL2832_FUNC1_STATE_CONFIG_2_WAIT,
	RTL2832_FUNC1_STATE_CONFIG_2_GET_BER,
	RTL2832_FUNC1_STATE_CONFIG_3_WAIT,
	RTL2832_FUNC1_STATE_CONFIG_3_GET_BER,
	RTL2832_FUNC1_STATE_DETERMINED_WAIT,
	RTL2832_FUNC1_STATE_DETERMINED,
}RTL2832_FUNC1_STATE;



// Manipulating functions
void
rtl2832_IsConnectedToI2c(
	
	int *pAnswer
);

int
rtl2832_SoftwareReset(

	);

int
rtl2832_Initialize(

	);
/*
int
rtl2832_SetBandwidthMode(
	
	int BandwidthMode
	);
*/
int
rtl2832_SetIfFreqHz(
	
	unsigned long IfFreqHz
	);

int
rtl2832_SetSpectrumMode(
	
	int SpectrumMode
	);

int
rtl2832_IsTpsLocked(
	
	int *pAnswer
	);

int
rtl2832_IsSignalLocked(
	
	int *pAnswer
	);

int
rtl2832_GetSignalStrength(
	
	unsigned long *pSignalStrength
	);

int
rtl2832_GetSignalQuality(
	
	unsigned long *pSignalQuality
	);

int
rtl2832_GetBer(
	
	unsigned long *pBerNum,
	unsigned long *pBerDen
	);

int
rtl2832_GetSnrDb(
	
	long *pSnrDbNum,
	long *pSnrDbDen
	);

int
rtl2832_GetRfAgc(
	
	long *pRfAgc
	);

int
rtl2832_GetIfAgc(
	
	long *pIfAgc
	);

int
rtl2832_GetDiAgc(
	
	unsigned char *pDiAgc
	);

int
rtl2832_GetTrOffsetPpm(
	
	long *pTrOffsetPpm
	);

int
rtl2832_GetCrOffsetHz(
	
	long *pCrOffsetHz
	);

int
rtl2832_GetConstellation(
	
	int *pConstellation
	);

int
rtl2832_GetHierarchy(
	
	int *pHierarchy
	);

int
rtl2832_GetCodeRateLp(
	
	int *pCodeRateLp
	);

int
rtl2832_GetCodeRateHp(
	
	int *pCodeRateHp
	);

int
rtl2832_GetGuardInterval(
	
	int *pGuardInterval
	);

int
rtl2832_GetFftMode(
	
	int *pFftMode
	);

int
rtl2832_UpdateFunction(

	);

int
rtl2832_ResetFunction(
	
	);








// RTL2832 extra functions
void
rtl2832_GetAppMode(
	
	int *pAppMode
	);





// RTL2832 dependence
int
rtl2832_func1_Reset(
	
	);

int
rtl2832_func1_Update(

	);

int
rtl2832_func1_IsCriterionMatched(
	
	int *pAnswer
	);

int
rtl2832_func1_AccumulateRsdBerEst(
	
	unsigned long *pAccumulativeValue
	);

int
rtl2832_func1_ResetReg(

	);

int
rtl2832_func1_SetCommonReg(

	);

int
rtl2832_func1_SetRegWithFftMode(
	
	int FftMode
	);

int
rtl2832_func1_SetRegWithConfigMode(
	
	int ConfigMode
	);

void
rtl2832_func1_GetMinWeightedBerConfigMode(
	
	int *pConfigMode
	);
















#endif
