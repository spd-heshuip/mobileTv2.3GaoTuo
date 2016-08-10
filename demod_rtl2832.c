/**

@file

@brief   RTL2832 demod module definition

One can manipulate RTL2832 demod through RTL2832 module.
RTL2832 module is derived from DVB-T demod module.

*/

#include "demod_rtl2832.h"


#include "math_mpi.h"
#include "stdio.h"


static int CurrentPageNo = 0;
static int CurBandwidthMode = 8000;
#define RTL2832U_Addr 0x20
#define I2cReadingByteNumMax 32
#define I2cWritingByteNumMax 32

extern int write_demod_register(
	unsigned char			demod_device_addr,		
	unsigned char			page,
	unsigned char			offset,
	unsigned char			*data,
	unsigned short		bytelength);

extern int read_demod_register(
	unsigned char			demod_device_addr,	
	unsigned char 		page,
	unsigned char 		offset,
	unsigned char*		data,
	unsigned short		bytelength);



	// RTL2832 update Function 1 variables
static	int Func1State = 0;

static	int Func1WaitTimeMax = 0;
static	int Func1GettingTimeMax = 0;
static	int Func1GettingNumEachTime = 0;

static	int Func1WaitTime = 0;
static	int Func1GettingTime = 0;

static	unsigned long Func1RsdBerEstSumNormal = 0;
static	unsigned long Func1RsdBerEstSumConfig1 = 0;
static	unsigned long Func1RsdBerEstSumConfig2 = 0;
static	unsigned long Func1RsdBerEstSumConfig3 = 0;

static	int Func1QamBak = 0;
static	int Func1HierBak = 0;
static	int Func1LpCrBak = 0;
static	int Func1HpCrBak = 0;
static	int Func1GiBak = 0;
static	int Func1FftBak = 0;

static DVBT_REG_ENTRY trl2832_RegTable[DVBT_REG_BIT_NAME_ITEM_TERMINATOR];

void rtl2832_InitRegTable()
{
	static const DVBT_PRIMARY_REG_ENTRY PrimaryRegTable[RTL2832_REG_TABLE_LEN] =
	{
		// Software reset register
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_SOFT_RST,						0x1,		0x1,			2,		2},

		// Tuner I2C forwording register
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_IIC_REPEAT,					0x1,		0x1,			3,		3},

		// Registers for initialization
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_TR_WAIT_MIN_8K,				0x1,		0x88,			11,		2},
		{DVBT_RSD_BER_FAIL_VAL,				0x1,		0x8f,			15,		0},
		{DVBT_EN_BK_TRK,					0x1,		0xa6,			7,		7},
		{DVBT_AD_EN_REG,					0x0,		0x8,			7,		7},
		{DVBT_AD_EN_REG1,					0x0,		0x8,			6,		6},
		{DVBT_EN_BBIN,						0x1,		0xb1,			0,		0},
		{DVBT_MGD_THD0,						0x1,		0x95,			7,		0},
		{DVBT_MGD_THD1,						0x1,		0x96,			7,		0},
		{DVBT_MGD_THD2,						0x1,		0x97,			7,		0},
		{DVBT_MGD_THD3,						0x1,		0x98,			7,		0},
		{DVBT_MGD_THD4,						0x1,		0x99,			7,		0},
		{DVBT_MGD_THD5,						0x1,		0x9a,			7,		0},
		{DVBT_MGD_THD6,						0x1,		0x9b,			7,		0},
		{DVBT_MGD_THD7,						0x1,		0x9c,			7,		0},
		{DVBT_EN_CACQ_NOTCH,				0x1,		0x61,			4,		4},
		{DVBT_AD_AV_REF,					0x0,		0x9,			6,		0},
		{DVBT_REG_PI,						0x0,		0xa,			2,		0},
		{DVBT_PIP_ON,						0x0,		0x21,			3,		3},
		{DVBT_SCALE1_B92,					0x2,		0x92,			7,		0},
		{DVBT_SCALE1_B93,					0x2,		0x93,			7,		0},
		{DVBT_SCALE1_BA7,					0x2,		0xa7,			7,		0},
		{DVBT_SCALE1_BA9,					0x2,		0xa9,			7,		0},
		{DVBT_SCALE1_BAA,					0x2,		0xaa,			7,		0},
		{DVBT_SCALE1_BAB,					0x2,		0xab,			7,		0},
		{DVBT_SCALE1_BAC,					0x2,		0xac,			7,		0},
		{DVBT_SCALE1_BB0,					0x2,		0xb0,			7,		0},
		{DVBT_SCALE1_BB1,					0x2,		0xb1,			7,		0},
		{DVBT_KB_P1,						0x1,		0x64,			3,		1},
		{DVBT_KB_P2,						0x1,		0x64,			6,		4},
		{DVBT_KB_P3,						0x1,		0x65,			2,		0},
		{DVBT_OPT_ADC_IQ,					0x0,		0x6,			5,		4},
		{DVBT_AD_AVI,						0x0,		0x9,			1,		0},
		{DVBT_AD_AVQ,						0x0,		0x9,			3,		2},
		{DVBT_K1_CR_STEP12,					0x2,		0xad,			9,		4},

		// Registers for initialization according to mode
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_TRK_KS_P2,					0x1,		0x6f,			2,		0},
		{DVBT_TRK_KS_I2,					0x1,		0x70,			5,		3},
		{DVBT_TR_THD_SET2,					0x1,		0x72,			3,		0},
		{DVBT_TRK_KC_P2,					0x1,		0x73,			5,		3},
		{DVBT_TRK_KC_I2,					0x1,		0x75,			2,		0},
		{DVBT_CR_THD_SET2,					0x1,		0x76,			7,		6},

		// Registers for IF setting
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_PSET_IFFREQ,					0x1,		0x19,			21,		0},
		{DVBT_SPEC_INV,						0x1,		0x15,			0,		0},

		// Registers for bandwidth programming
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_RSAMP_RATIO,					0x1,		0x9f,			27,		2},
		{DVBT_CFREQ_OFF_RATIO,				0x1,		0x9d,			23,		4},

		// FSM stage register
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_FSM_STAGE,					0x3,		0x51,			6,		3},

		// TPS content registers
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_RX_CONSTEL,					0x3,		0x3c,			3,		2},
		{DVBT_RX_HIER,						0x3,		0x3c,			6,		4},
		{DVBT_RX_C_RATE_LP,					0x3,		0x3d,			2,		0},
		{DVBT_RX_C_RATE_HP,					0x3,		0x3d,			5,		3},
		{DVBT_GI_IDX,						0x3,		0x51,			1,		0},
		{DVBT_FFT_MODE_IDX,					0x3,		0x51,			2,		2},

		// Performance measurement registers
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_RSD_BER_EST,					0x3,		0x4e,			15,		0},
		{DVBT_CE_EST_EVM,					0x4,		0xc,			15,		0},

		// AGC registers
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_RF_AGC_VAL,					0x3,		0x5b,			13,		0},
		{DVBT_IF_AGC_VAL,					0x3,		0x59,			13,		0},
		{DVBT_DAGC_VAL,						0x3,		0x5,			7,		0},

		// TR offset and CR offset registers
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_SFREQ_OFF,					0x3,		0x18,			13,		0},
		{DVBT_CFREQ_OFF,					0x3,		0x5f,			17,		0},

		// AGC relative registers
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_POLAR_RF_AGC,					0x0,		0xe,			1,		1},
		{DVBT_POLAR_IF_AGC,					0x0,		0xe,			0,		0},
		{DVBT_AAGC_HOLD,					0x1,		0x4,			5,		5},
		{DVBT_EN_RF_AGC,					0x1,		0x4,			6,		6},
		{DVBT_EN_IF_AGC,					0x1,		0x4,			7,		7},
		{DVBT_IF_AGC_MIN,					0x1,		0x8,			7,		0},
		{DVBT_IF_AGC_MAX,					0x1,		0x9,			7,		0},
		{DVBT_RF_AGC_MIN,					0x1,		0xa,			7,		0},
		{DVBT_RF_AGC_MAX,					0x1,		0xb,			7,		0},
		{DVBT_IF_AGC_MAN,					0x1,		0xc,			6,		6},
		{DVBT_IF_AGC_MAN_VAL,				0x1,		0xc,			13,		0},
		{DVBT_RF_AGC_MAN,					0x1,		0xe,			6,		6},
		{DVBT_RF_AGC_MAN_VAL,				0x1,		0xe,			13,		0},
		{DVBT_DAGC_TRG_VAL,					0x1,		0x12,			7,		0},
		{DVBT_AGC_TARG_VAL_0,				0x1,		0x2,			0,		0},
		{DVBT_AGC_TARG_VAL_8_1,				0x1,		0x3,			7,		0},
		{DVBT_AAGC_LOOP_GAIN,				0x1,		0xc7,			5,		1},
		{DVBT_LOOP_GAIN2_3_0,				0x1,		0x4,			4,		1},
		{DVBT_LOOP_GAIN2_4,					0x1,		0x5,			7,		7},
		{DVBT_LOOP_GAIN3,					0x1,		0xc8,			4,		0},
		{DVBT_VTOP1,						0x1,		0x6,			5,		0},
		{DVBT_VTOP2,						0x1,		0xc9,			5,		0},
		{DVBT_VTOP3,						0x1,		0xca,			5,		0},
		{DVBT_KRF1,							0x1,		0xcb,			7,		0},
		{DVBT_KRF2,							0x1,		0x7,			7,		0},
		{DVBT_KRF3,							0x1,		0xcd,			7,		0},
		{DVBT_KRF4,							0x1,		0xce,			7,		0},
		{DVBT_EN_GI_PGA,					0x1,		0xe5,			0,		0},
		{DVBT_THD_LOCK_UP,					0x1,		0xd9,			8,		0},
		{DVBT_THD_LOCK_DW,					0x1,		0xdb,			8,		0},
		{DVBT_THD_UP1,						0x1,		0xdd,			7,		0},
		{DVBT_THD_DW1,						0x1,		0xde,			7,		0},
		{DVBT_INTER_CNT_LEN,				0x1,		0xd8,			3,		0},
		{DVBT_GI_PGA_STATE,					0x1,		0xe6,			3,		3},
		{DVBT_EN_AGC_PGA,					0x1,		0xd7,			0,		0},

		// TS interface registers
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_CKOUTPAR,						0x1,		0x7b,			5,		5},
		{DVBT_CKOUT_PWR,					0x1,		0x7b,			6,		6},
		{DVBT_SYNC_DUR,						0x1,		0x7b,			7,		7},
		{DVBT_ERR_DUR,						0x1,		0x7c,			0,		0},
		{DVBT_SYNC_LVL,						0x1,		0x7c,			1,		1},
		{DVBT_ERR_LVL,						0x1,		0x7c,			2,		2},
		{DVBT_VAL_LVL,						0x1,		0x7c,			3,		3},
		{DVBT_SERIAL,						0x1,		0x7c,			4,		4},
		{DVBT_SER_LSB,						0x1,		0x7c,			5,		5},
		{DVBT_CDIV_PH0,						0x1,		0x7d,			3,		0},
		{DVBT_CDIV_PH1,						0x1,		0x7d,			7,		4},
		{DVBT_MPEG_IO_OPT_2_2,				0x0,		0x6,			7,		7},
		{DVBT_MPEG_IO_OPT_1_0,				0x0,		0x7,			7,		6},
		{DVBT_CKOUTPAR_PIP,					0x0,		0xb7,			4,		4},
		{DVBT_CKOUT_PWR_PIP,				0x0,		0xb7,			3,		3},
		{DVBT_SYNC_LVL_PIP,					0x0,		0xb7,			2,		2},
		{DVBT_ERR_LVL_PIP,					0x0,		0xb7,			1,		1},
		{DVBT_VAL_LVL_PIP,					0x0,		0xb7,			0,		0},
		{DVBT_CKOUTPAR_PID,					0x0,		0xb9,			4,		4},
		{DVBT_CKOUT_PWR_PID,				0x0,		0xb9,			3,		3},
		{DVBT_SYNC_LVL_PID,					0x0,		0xb9,			2,		2},
		{DVBT_ERR_LVL_PID,					0x0,		0xb9,			1,		1},
		{DVBT_VAL_LVL_PID,					0x0,		0xb9,			0,		0},

		// FSM state-holding register
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_SM_PASS,						0x1,		0x93,			11,		0},

		// AD7 registers
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_AD7_SETTING,					0x0,		0x11,			15,		0},
		{DVBT_RSSI_R,						0x3,		0x1,			6,		0},

		// ACI detection registers
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_ACI_DET_IND,					0x3,		0x12,			0,		0},

		// Clock output registers
		// RegBitName,						PageNo,		RegStartAddr,	MSB,	LSB
		{DVBT_REG_MON,						0x0,		0xd,			1,		0},
		{DVBT_REG_MONSEL,					0x0,		0xd,			2,		2},
		{DVBT_REG_GPE,						0x0,		0xd,			7,		7},
		{DVBT_REG_GPO,						0x0,		0x10,			0,		0},
		{DVBT_REG_4MSEL,					0x0,		0x13,			0,		0},
	};


	int i;
	int RegBitName=0;

	// Initialize register table according to primary register table.
	// Note: 1. Register table rows are sorted by register bit name key.
	//       2. The default value of the IsAvailable variable is "NO".
	for(i = 0; i < DVBT_REG_TABLE_LEN_MAX; i++)
		trl2832_RegTable[i].IsAvailable  = NO;

	for(i = 0; i < RTL2832_REG_TABLE_LEN; i++)
	{
		RegBitName = PrimaryRegTable[i].RegBitName;

		trl2832_RegTable[RegBitName].IsAvailable  = YES;
		trl2832_RegTable[RegBitName].PageNo       = PrimaryRegTable[i].PageNo;
		trl2832_RegTable[RegBitName].RegStartAddr = PrimaryRegTable[i].RegStartAddr;
		trl2832_RegTable[RegBitName].Msb          = PrimaryRegTable[i].Msb;
		trl2832_RegTable[RegBitName].Lsb          = PrimaryRegTable[i].Lsb;
	}

	return;
}

int rtl2832_GetBandwidthMode(int *pBandwidthMode)
{
	 *pBandwidthMode = CurBandwidthMode;
	return 0;
}


int SetRegPage(int PageNo)
{
	CurrentPageNo = PageNo;
	return 0;
}	

int GetRegBytes(int RegStartAddr, unsigned char *pReadingBytes,int ByteNum)
{
	
	unsigned int i;
	unsigned char DeviceAddr;
	unsigned char ReadingByteNum, ReadingByteNumMax, ReadingByteNumRem;
	unsigned char RegReadingAddr;

	DeviceAddr = RTL2832U_Addr;
	ReadingByteNumMax = I2cReadingByteNumMax;

	// Get demod register bytes.
	// Note: Get demod register bytes considering maximum reading byte number.
	for(i = 0; i < ByteNum; i += ReadingByteNumMax)
	{
		// Set register reading address.
		RegReadingAddr = RegStartAddr + i;

		// Calculate remainder reading byte number.
		ReadingByteNumRem = ByteNum - i;

		// Determine reading byte number.
		ReadingByteNum = (ReadingByteNumRem > ReadingByteNumMax) ? ReadingByteNumMax : ReadingByteNumRem;

		// Get demod register bytes.
		if(read_demod_register( DeviceAddr, CurrentPageNo, RegReadingAddr, &pReadingBytes[i], ReadingByteNum)) return 1;

	}

	return 0;
}

int SetRegBytes(int RegStartAddr, unsigned char *pWritingBytes,int ByteNum)
{	
	unsigned int i, j;

	unsigned char DeviceAddr;
	unsigned char WritingBuffer[I2C_BUFFER_LEN];
	unsigned char WritingByteNum, WritingByteNumMax, WritingByteNumRem;
	unsigned char RegWritingAddr;

	DeviceAddr = RTL2832U_Addr;

	// Calculate maximum writing byte number.
	WritingByteNumMax = I2cWritingByteNumMax - LEN_1_BYTE;


	// Set demod register bytes with writing bytes.
	// Note: Set demod register bytes considering maximum writing byte number.
	for(i = 0; i < ByteNum; i += WritingByteNumMax)
	{
		// Set register writing address.
		RegWritingAddr = RegStartAddr + i;

		// Calculate remainder writing byte number.
		WritingByteNumRem = ByteNum - i;

		// Determine writing byte number.
		WritingByteNum = (WritingByteNumRem > WritingByteNumMax) ? WritingByteNumMax : WritingByteNumRem;


		for(j = 0; j < WritingByteNum; j++)
			WritingBuffer[j] = pWritingBytes[i + j];

		// Set demod register bytes with writing buffer.
		if(write_demod_register( DeviceAddr,CurrentPageNo, RegWritingAddr, WritingBuffer, WritingByteNum )) return 1;

		
	}


	return 0;
}

int
SetRegMaskBits(
	unsigned char RegStartAddr,
	unsigned char Msb,
	unsigned char Lsb,
	const unsigned long WritingValue
	)
{
	int i;

	unsigned char ReadingBytes[LEN_4_BYTE];
	unsigned char WritingBytes[LEN_4_BYTE];

	unsigned char ByteNum;
	unsigned long Mask;
	unsigned char Shift;

	unsigned long Value;


	// Calculate writing byte number according to MSB.
	ByteNum = Msb / BYTE_BIT_NUM + LEN_1_BYTE;


	// Generate mask and shift according to MSB and LSB.
	Mask = 0;

	for(i = Lsb; i < (unsigned char)(Msb + 1); i++)
		Mask |= 0x1 << i;

	Shift = Lsb;


	// Get demod register bytes according to register start adddress and byte number.
	if(GetRegBytes( RegStartAddr, ReadingBytes, ByteNum) != 0)
		return 1;

	// Combine reading bytes into an unsigned integer value.
	// Note: Put lower address byte on value MSB.
	//       Put upper address byte on value LSB.
	Value = 0;

	for(i = 0; i < ByteNum; i++)
		Value |= (unsigned long)ReadingBytes[i] << (BYTE_SHIFT * (ByteNum - i -1));

	// Reserve unsigned integer value unmask bit with mask and inlay writing value into it.
	Value &= ~Mask;
	Value |= (WritingValue << Shift) & Mask;

	// Separate unsigned integer value into writing bytes.
	// Note: Pick up lower address byte from value MSB.
	//       Pick up upper address byte from value LSB.
	for(i = 0; i < ByteNum; i++)
		WritingBytes[i] = (unsigned char)((Value >> (BYTE_SHIFT * (ByteNum - i -1))) & BYTE_MASK);


	// Write demod register bytes with writing bytes.
	if(SetRegBytes( RegStartAddr, WritingBytes, ByteNum) != 0)
		return 1;



	return 0;
}


int
GetRegMaskBits(
	unsigned char RegStartAddr,
	unsigned char Msb,
	unsigned char Lsb,
	unsigned long *pReadingValue
	)
{
	int i;

	unsigned char ReadingBytes[LEN_4_BYTE];

	unsigned char ByteNum;
	unsigned long Mask;
	unsigned char Shift;

	unsigned long Value;


	// Calculate writing byte number according to MSB.
	ByteNum = Msb / BYTE_BIT_NUM + LEN_1_BYTE;


	// Generate mask and shift according to MSB and LSB.
	Mask = 0;

	for(i = Lsb; i < (unsigned char)(Msb + 1); i++)
		Mask |= 0x1 << i;

	Shift = Lsb;


	// Get demod register bytes according to register start adddress and byte number.
	if(GetRegBytes(RegStartAddr, ReadingBytes, ByteNum) != 0)
		return 1;


	// Combine reading bytes into an unsigned integer value.
	// Note: Put lower address byte on value MSB.
	//       Put upper address byte on value LSB.
	Value = 0;

	for(i = 0; i < ByteNum; i++)
		Value |= (unsigned long)ReadingBytes[i] << (BYTE_SHIFT * (ByteNum - i -1));


	// Get register bits from unsigned integaer value with mask and shift
	*pReadingValue = (Value & Mask) >> Shift;


	return 0;


}


int SetRegBitsWithPage(int nRegBitName,unsigned long WritingValue)
{

	int i;

	unsigned char ReadingBytes[LEN_4_BYTE];
	unsigned char WritingBytes[LEN_4_BYTE];

	unsigned char ByteNum;
	unsigned long Mask;
	unsigned char Shift;

	unsigned long Value;

	unsigned char _RegStartAddr;
	unsigned char Msb;
	unsigned char Lsb;

	// Check if register bit name is available.
	if(trl2832_RegTable[nRegBitName].IsAvailable == NO)
		return 1;

	// Get register start address, MSB, and LSB from register table with register bit name key.
	_RegStartAddr = trl2832_RegTable[nRegBitName].RegStartAddr;
	Msb          = trl2832_RegTable[nRegBitName].Msb;
	Lsb          = trl2832_RegTable[nRegBitName].Lsb;

	SetRegPage(trl2832_RegTable[nRegBitName].PageNo);
	
	// Calculate writing byte number according to MSB.
	ByteNum = Msb / BYTE_BIT_NUM + LEN_1_BYTE;


	// Generate mask and shift according to MSB and LSB.
	Mask = 0;

	for(i = Lsb; i < (unsigned char)(Msb + 1); i++)
		Mask |= 0x1 << i;

	Shift = Lsb;


	// Get demod register bytes according to register start adddress and byte number.
	if(GetRegBytes( _RegStartAddr, ReadingBytes, ByteNum) != 0)
		return 1;


	// Combine reading bytes into an unsigned integer value.
	// Note: Put lower address byte on value MSB.
	//       Put upper address byte on value LSB.
	Value = 0;

	for(i = 0; i < ByteNum; i++)
		Value |= (unsigned long)ReadingBytes[i] << (BYTE_SHIFT * (ByteNum - i -1));


	// Reserve unsigned integer value unmask bit with mask and inlay writing value into it.
	Value &= ~Mask;
	Value |= (WritingValue << Shift) & Mask;


	// Separate unsigned integer value into writing bytes.
	// Note: Pick up lower address byte from value MSB.
	//       Pick up upper address byte from value LSB.
	for(i = 0; i < ByteNum; i++)
		WritingBytes[i] = (unsigned char)((Value >> (BYTE_SHIFT * (ByteNum - i -1))) & BYTE_MASK);


	// Write demod register bytes with writing bytes.
	if(SetRegBytes(_RegStartAddr, WritingBytes, ByteNum) != 0)
		return 1;

	return 0;
}



int GetRegBitsWithPage(int nRegBitName,unsigned long *pReadingValue)
{

	int i;

	unsigned char ReadingBytes[LEN_4_BYTE];

	unsigned char ByteNum;
	unsigned long Mask;
	unsigned char Shift;

	unsigned long Value;
	
	unsigned char _RegStartAddr;
	unsigned char Msb;
	unsigned char Lsb;

		// Check if register bit name is available.
	if(trl2832_RegTable[nRegBitName].IsAvailable == NO)
			return 1;

		// Get register start address, MSB, and LSB from register table with register bit name key.
	_RegStartAddr = trl2832_RegTable[nRegBitName].RegStartAddr;
	Msb          = trl2832_RegTable[nRegBitName].Msb;
	Lsb          = trl2832_RegTable[nRegBitName].Lsb;

	SetRegPage(trl2832_RegTable[nRegBitName].PageNo);

	// Calculate writing byte number according to MSB.
	ByteNum = Msb / BYTE_BIT_NUM + LEN_1_BYTE;


	// Generate mask and shift according to MSB and LSB.
	Mask = 0;

	for(i = Lsb; i < (unsigned char)(Msb + 1); i++)
		Mask |= 0x1 << i;

	Shift = Lsb;


	// Get demod register bytes according to register start adddress and byte number.
	if(GetRegBytes( _RegStartAddr, ReadingBytes, ByteNum) != 0)
		return 1;


	// Combine reading bytes into an unsigned integer value.
	// Note: Put lower address byte on value MSB.
	//       Put upper address byte on value LSB.
	Value = 0;

	for(i = 0; i < ByteNum; i++)
		Value |= (unsigned long)ReadingBytes[i] << (BYTE_SHIFT * (ByteNum - i -1));


	// Get register bits from unsigned integaer value with mask and shift
	*pReadingValue = (Value & Mask) >> Shift;

	return 0;
}




long
BinToSignedInt(
	unsigned long Binary,
	unsigned char BitNum
	)
{
	int i;

	unsigned char SignedBit;
	unsigned long SignedBitExtension;

	long Value;



	// Get signed bit.
	SignedBit = (unsigned char)((Binary >> (BitNum - 1)) & BIT_0_MASK);


	// Generate signed bit extension.
	SignedBitExtension = 0;

	for(i = BitNum; i < LONG_BIT_NUM; i++)
		SignedBitExtension |= SignedBit << i;


	// Combine binary value and signed bit extension to signed integer value.
	Value = (long)(Binary | SignedBitExtension);


	return Value;
}




unsigned long
SignedIntToBin(
	long Value,
	unsigned char BitNum
	)
{
	unsigned int i;
	unsigned long Mask, Binary;



	// Generate Mask according to BitNum.
	Mask = 0;
	for(i = 0; i < BitNum; i++)
		Mask |= 0x1 << i;


	// Convert signed integer to binary with Mask.
	Binary = Value & Mask;


	return Binary;
}



/**

@see   DVBT_DEMOD_FP_SOFTWARE_RESET

*/
int rtl2832_SoftwareReset(void)
{
	// Set SOFT_RST with 1. Then, set SOFT_RST with 0.
	if(SetRegBitsWithPage( DVBT_SOFT_RST, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	if(SetRegBitsWithPage( DVBT_SOFT_RST, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	return FUNCTION_SUCCESS;
error_status_set_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_INITIALIZE

*/
int
rtl2832_Initialize()
{
	// Initializing table entry only used in Initialize()
	typedef struct
	{
		int RegBitName;
		unsigned long WritingValue;
	}
	INIT_TABLE_ENTRY;

	// TS interface initializing table entry only used in Initialize()
	typedef struct
	{
		int RegBitName;
		unsigned long WritingValue[TS_INTERFACE_MODE_NUM];
	}
	TS_INTERFACE_INIT_TABLE_ENTRY;

	// Application initializing table entry only used in Initialize()
	typedef struct
	{
		int RegBitName;
		unsigned long WritingValue[RTL2832_APPLICATION_MODE_NUM];
	}
	APP_INIT_TABLE_ENTRY;



	static const INIT_TABLE_ENTRY InitTable[RTL2832_INIT_TABLE_LEN] =
	{
		// RegBitName,				WritingValue
		{DVBT_AD_EN_REG,			0x1		},
		{DVBT_AD_EN_REG1,			0x1		},
		{DVBT_RSD_BER_FAIL_VAL,		0x2800	},
		{DVBT_MGD_THD0,				0x10	},
		{DVBT_MGD_THD1,				0x20	},
		{DVBT_MGD_THD2,				0x20	},
		{DVBT_MGD_THD3,				0x40	},
		{DVBT_MGD_THD4,				0x22	},
		{DVBT_MGD_THD5,				0x32	},
		{DVBT_MGD_THD6,				0x37	},
		{DVBT_MGD_THD7,				0x39	},
		{DVBT_EN_BK_TRK,			0x0		},
		{DVBT_EN_CACQ_NOTCH,		0x0		},
		{DVBT_AD_AV_REF,			0x2a	},
		{DVBT_REG_PI,				0x6		},
		{DVBT_PIP_ON,				0x0		},
		{DVBT_CDIV_PH0,				0x8		},
		{DVBT_CDIV_PH1,				0x8		},
		{DVBT_SCALE1_B92,			0x4		},
		{DVBT_SCALE1_B93,			0xb0	},
		{DVBT_SCALE1_BA7,			0x78	},
		{DVBT_SCALE1_BA9,			0x28	},
		{DVBT_SCALE1_BAA,			0x59	},
		{DVBT_SCALE1_BAB,			0x83	},
		{DVBT_SCALE1_BAC,			0xd4	},
		{DVBT_SCALE1_BB0,			0x65	},
		{DVBT_SCALE1_BB1,			0x43	},
		{DVBT_KB_P1,				0x1		},
		{DVBT_KB_P2,				0x4		},
		{DVBT_KB_P3,				0x7		},
		{DVBT_K1_CR_STEP12,			0xa		},
		{DVBT_REG_GPE,				0x1		},
	};

	static const TS_INTERFACE_INIT_TABLE_ENTRY TsInterfaceInitTable[RTL2832_TS_INTERFACE_INIT_TABLE_LEN] =
	{
		// RegBitName,				WritingValue for {Parallel, Serial}
		{DVBT_SERIAL,				{0x0,	0x1}},
		{DVBT_CDIV_PH0,				{0x9,	0x1}},
		{DVBT_CDIV_PH1,				{0x9,	0x2}},
		{DVBT_MPEG_IO_OPT_2_2,		{0x0,	0x0}},
		{DVBT_MPEG_IO_OPT_1_0,		{0x0,	0x1}},
	};

	static const APP_INIT_TABLE_ENTRY AppInitTable[RTL2832_APP_INIT_TABLE_LEN] =
	{
		// RegBitName,				WritingValue for {Dongle, STB}
		{DVBT_TRK_KS_P2,			{0x4,	0x4}},
		{DVBT_TRK_KS_I2,			{0x7,	0x7}},
		{DVBT_TR_THD_SET2,			{0x6,	0x6}},
		{DVBT_TRK_KC_I2,			{0x5,	0x6}},
		{DVBT_CR_THD_SET2,			{0x1,	0x1}},
	};


	int i;

	int TsInterfaceMode=TS_INTERFACE_SERIAL;//TS_INTERFACE_PARALLEL;
	int AppMode=RTL2832_APPLICATION_DONGLE;

	rtl2832_InitRegTable();

	// Initialize demod registers according to the initializing table.
	for(i = 0; i < RTL2832_INIT_TABLE_LEN; i++)
	{
		
		if(SetRegBitsWithPage(InitTable[i].RegBitName, InitTable[i].WritingValue)!= FUNCTION_SUCCESS)
			goto error_status_set_registers;
	}

	// Initialize demod registers according to the TS interface initializing table.
	for(i = 0; i < RTL2832_TS_INTERFACE_INIT_TABLE_LEN; i++)
	{
		if(SetRegBitsWithPage( TsInterfaceInitTable[i].RegBitName,TsInterfaceInitTable[i].WritingValue[TsInterfaceMode]) != FUNCTION_SUCCESS)
			goto error_status_set_registers;
	}

	// Initialize demod registers according to the application initializing table.
	for(i = 0; i < RTL2832_APP_INIT_TABLE_LEN; i++)
	{
		if(SetRegBitsWithPage(AppInitTable[i].RegBitName,AppInitTable[i].WritingValue[AppMode]) != FUNCTION_SUCCESS)
			goto error_status_set_registers;
	}

	return FUNCTION_SUCCESS;


error_status_set_registers:
	return FUNCTION_ERROR;
}


static int rtl2832_GetCrystalFreqHz(unsigned long *CrystalFreqHz)
{
//	*CrystalFreqHz = CRYSTAL_FREQ_28800000HZ;
	*CrystalFreqHz = CRYSTAL_FREQ_16000000HZ;

	return 0;	
}


/**

@see   DVBT_DEMOD_FP_SET_BANDWIDTH_MODE

*/


int
rtl2832_SetBandwidthMode(
	DVBT_BANDWIDTH_MODE BandwidthMode
	)
{
	static  unsigned char HlpfxTable[DVBT_BANDWIDTH_MODE_NUM][RTL2832_H_LPF_X_LEN] =
	{
		// H_LPF_X writing value for 6 MHz bandwidth
		{
			0xf5,	0xff,	0x15,	0x38,	0x5d,	0x6d,	0x52,	0x07,	0xfa,	0x2f,
			0x53,	0xf5,	0x3f,	0xca,	0x0b,	0x91,	0xea,	0x30,	0x63,	0xb2,
			0x13,	0xda,	0x0b,	0xc4,	0x18,	0x7e,	0x16,	0x66,	0x08,	0x67,
			0x19,	0xe0,
		},

		// H_LPF_X writing value for 7 MHz bandwidth
		{
			0xe7,	0xcc,	0xb5,	0xba,	0xe8,	0x2f,	0x67,	0x61,	0x00,	0xaf,
			0x86,	0xf2,	0xbf,	0x59,	0x04,	0x11,	0xb6,	0x33,	0xa4,	0x30,
			0x15,	0x10,	0x0a,	0x42,	0x18,	0xf8,	0x17,	0xd9,	0x07,	0x22,
			0x19,	0x10,
		},

		// H_LPF_X writing value for 8 MHz bandwidth
		{
			0x09,	0xf6,	0xd2,	0xa7,	0x9a,	0xc9,	0x27,	0x77,	0x06,	0xbf,
			0xec,	0xf4,	0x4f,	0x0b,	0xfc,	0x01,	0x63,	0x35,	0x54,	0xa7,
			0x16,	0x66,	0x08,	0xb4,	0x19,	0x6e,	0x19,	0x65,	0x05,	0xc8,
			0x19,	0xe0,
		},
	};


	unsigned long CrystalFreqHz;

	long ConstWithBandwidthMode;

	MPI MpiCrystalFreqHz;
	MPI MpiConst, MpiVar0, MpiVar1, MpiNone;

	unsigned long RsampRatio;

	long CfreqOffRatioInt;
	unsigned long CfreqOffRatioBinary;



	// Get demod crystal frequency in Hz.
	rtl2832_GetCrystalFreqHz( &CrystalFreqHz);


	// Set H_LPF_X registers with HlpfxTable according to BandwidthMode.
	if(SetRegPage(RTL2832_H_LPF_X_PAGE) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	if(SetRegBytes(RTL2832_H_LPF_X_ADDR, HlpfxTable[BandwidthMode], RTL2832_H_LPF_X_LEN) !=
		FUNCTION_SUCCESS)
		goto error_status_set_registers;


	// Determine constant value with bandwidth mode.
	switch(BandwidthMode)
	{
		default:
		case DVBT_BANDWIDTH_6MHZ:	ConstWithBandwidthMode = 48000000;		break;
		case DVBT_BANDWIDTH_7MHZ:	ConstWithBandwidthMode = 56000000;		break;
		case DVBT_BANDWIDTH_8MHZ:	ConstWithBandwidthMode = 64000000;		break;
	}


	// Calculate RSAMP_RATIO value.
	// Note: RSAMP_RATIO = floor(CrystalFreqHz * 7 * pow(2, 22) / ConstWithBandwidthMode)
	MpiSetValue(&MpiCrystalFreqHz, CrystalFreqHz);
	MpiSetValue(&MpiVar1,          ConstWithBandwidthMode);
	MpiSetValue(&MpiConst,         7);

	MpiMul(&MpiVar0, MpiCrystalFreqHz, MpiConst);
	MpiLeftShift(&MpiVar0, MpiVar0, 22);
	MpiDiv(&MpiVar0, &MpiNone, MpiVar0, MpiVar1);

	MpiGetValue(MpiVar0, (long *)&RsampRatio);


	// Set RSAMP_RATIO with calculated value.
	// Note: Use SetRegBitsWithPage() to set register bits with page setting.
	if(SetRegBitsWithPage( DVBT_RSAMP_RATIO, RsampRatio) != FUNCTION_SUCCESS)
		goto error_status_set_registers;


	// Calculate CFREQ_OFF_RATIO value.
	// Note: CFREQ_OFF_RATIO = - floor(ConstWithBandwidthMode * pow(2, 20) / (CrystalFreqHz * 7))
	MpiSetValue(&MpiCrystalFreqHz, CrystalFreqHz);
	MpiSetValue(&MpiVar0,          ConstWithBandwidthMode);
	MpiSetValue(&MpiConst,         7);

	MpiLeftShift(&MpiVar0, MpiVar0, 20);
	MpiMul(&MpiVar1, MpiCrystalFreqHz, MpiConst);
	MpiDiv(&MpiVar0, &MpiNone, MpiVar0, MpiVar1);

	MpiGetValue(MpiVar0, &CfreqOffRatioInt);
	CfreqOffRatioInt = - CfreqOffRatioInt;

	CfreqOffRatioBinary = SignedIntToBin(CfreqOffRatioInt, RTL2832_CFREQ_OFF_RATIO_BIT_NUM);


	// Set CFREQ_OFF_RATIO with calculated value.
	// Note: Use SetRegBitsWithPage() to set register bits with page setting.
	if(SetRegBitsWithPage( DVBT_CFREQ_OFF_RATIO, CfreqOffRatioBinary) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	CurBandwidthMode = BandwidthMode;


	return FUNCTION_SUCCESS;


error_status_set_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_SET_IF_FREQ_HZ

*/
int
rtl2832_SetIfFreqHz(unsigned long IfFreqHz)
{
	unsigned long CrystalFreqHz;

	unsigned long EnBbin;

	MPI MpiCrystalFreqHz, MpiVar, MpiNone;

	long PsetIffreqInt;
	unsigned long PsetIffreqBinary;

	// Get demod crystal frequency in Hz.
	rtl2832_GetCrystalFreqHz( &CrystalFreqHz);

	// Determine and set EN_BBIN value.
	EnBbin = (IfFreqHz == IF_FREQ_0HZ) ? 0x1 : 0x0;

	if(SetRegBitsWithPage(DVBT_EN_BBIN, EnBbin) != FUNCTION_SUCCESS)
		goto error_status_set_registers;


	// Calculate PSET_IFFREQ value.
	// Note: PSET_IFFREQ = - floor((IfFreqHz % CrystalFreqHz) * pow(2, 22) / CrystalFreqHz)
	MpiSetValue(&MpiCrystalFreqHz, CrystalFreqHz);

	MpiSetValue(&MpiVar, (IfFreqHz % CrystalFreqHz));
	MpiLeftShift(&MpiVar, MpiVar, RTL2832_PSET_IFFREQ_BIT_NUM);
	MpiDiv(&MpiVar, &MpiNone, MpiVar, MpiCrystalFreqHz);

	MpiGetValue(MpiVar, &PsetIffreqInt);
	PsetIffreqInt = - PsetIffreqInt;

	PsetIffreqBinary = SignedIntToBin(PsetIffreqInt, RTL2832_PSET_IFFREQ_BIT_NUM);


	// Set PSET_IFFREQ with calculated value.
	// Note: Use SetRegBitsWithPage() to set register bits with page setting.
	if(SetRegBitsWithPage( DVBT_PSET_IFFREQ, PsetIffreqBinary) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	return FUNCTION_SUCCESS;


error_status_set_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_SET_SPECTRUM_MODE

*/
int
rtl2832_SetSpectrumMode(
	int SpectrumMode
	)
{
	unsigned long SpecInv;



	// Determine SpecInv according to spectrum mode.
	switch(SpectrumMode)
	{
		default:
		case SPECTRUM_NORMAL:		SpecInv = 0;		break;
		case SPECTRUM_INVERSE:		SpecInv = 1;		break;
	}


	// Set SPEC_INV with SpecInv.
	if(SetRegBitsWithPage( DVBT_SPEC_INV, SpecInv) != FUNCTION_SUCCESS)
		goto error_status_set_registers;


	return FUNCTION_SUCCESS;


error_status_set_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_IS_TPS_LOCKED

*/
int rtl2832_IsTpsLocked(int *pAnswer)
{
	unsigned long FsmStage;

	// Get FSM stage from FSM_STAGE.
	if(GetRegBitsWithPage(DVBT_FSM_STAGE, &FsmStage) != FUNCTION_SUCCESS)
		goto error_status_get_registers;


	// Determine answer according to FSM stage.
	if(FsmStage > 9)
		*pAnswer = YES;
	else
		*pAnswer = NO;


	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_IS_SIGNAL_LOCKED

*/
int rtl2832_IsSignalLocked(	int *pAnswer)
{
	unsigned long FsmStage;
//	int t;

	// rtl2832_IsTpsLocked(&t);

	// Get FSM stage from FSM_STAGE.
	if(GetRegBitsWithPage(DVBT_FSM_STAGE, &FsmStage) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	// Determine answer according to FSM stage.
	if(FsmStage == 11)
		*pAnswer = YES;
	else
		*pAnswer = NO;


	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_SIGNAL_STRENGTH

*/
int
rtl2832_GetSignalStrength(
	unsigned long *pSignalStrength
	)
{
	unsigned long FsmStage;
	long IfAgc;



	// Get FSM stage and IF AGC value.
	if(GetRegBitsWithPage( DVBT_FSM_STAGE, &FsmStage) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	if(rtl2832_GetIfAgc( &IfAgc) != FUNCTION_SUCCESS)
		goto error_status_get_registers;


	//  Determine signal strength according to FSM stage and IF AGC value.
	if(FsmStage < 10)
		*pSignalStrength = 0;
	else
		*pSignalStrength = 55 - IfAgc / 182;


	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_SIGNAL_QUALITY

*/
int
rtl2832_GetSignalQuality(
	unsigned long *pSignalQuality
	)
{
	unsigned long FsmStage, RsdBerEst;

	MPI MpiVar;
	long Var;



	// Get FSM_STAGE and RSD_BER_EST.
	if(GetRegBitsWithPage( DVBT_FSM_STAGE, &FsmStage) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	if(GetRegBitsWithPage(DVBT_RSD_BER_EST, &RsdBerEst) != FUNCTION_SUCCESS)
		goto error_status_get_registers;


	// If demod is not signal-locked, set signal quality with zero.
	if(FsmStage < 10)
	{
		*pSignalQuality = 0;
		goto success_status_non_signal_lock;
	}

	// Determine signal quality according to RSD_BER_EST.
	// Note: Map RSD_BER_EST value 8192 ~ 128 to 10 ~ 100
	//       Original formula: SignalQuality = 205 - 15 * log2(RSD_BER_EST)
	//       Adjusted formula: SignalQuality = ((205 << 5) - 15 * (log2(RSD_BER_EST) << 5)) >> 5
	//       If RSD_BER_EST > 8192, signal quality is 10.
	//       If RSD_BER_EST < 128, signal quality is 100.
	if(RsdBerEst > 8192)
	{
		*pSignalQuality = 10;
	}
	else if(RsdBerEst < 128)
	{
		*pSignalQuality = 100;
	}
	else
	{
		MpiSetValue(&MpiVar, RsdBerEst);
		MpiLog2(&MpiVar, MpiVar, RTL2832_SQ_FRAC_BIT_NUM);
		MpiGetValue(MpiVar, &Var);

		*pSignalQuality = ((205 << RTL2832_SQ_FRAC_BIT_NUM) - 15 * Var) >> RTL2832_SQ_FRAC_BIT_NUM;
	}


success_status_non_signal_lock:
	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_BER

*/
int rtl2832_GetBer(
	unsigned long *pBerNum,
	unsigned long *pBerDen
	)
{
	unsigned long RsdBerEst;

	// Get RSD_BER_EST.
	if(GetRegBitsWithPage( DVBT_RSD_BER_EST, &RsdBerEst) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	// Set BER numerator according to RSD_BER_EST.
	*pBerNum = RsdBerEst;

	// Set BER denominator.
	*pBerDen = RTL2832_BER_DEN_VALUE;

	return FUNCTION_SUCCESS;

error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_SNR_DB

*/
int rtl2832_GetSnrDb(
	long *pSnrDbNum,
	long *pSnrDbDen
	)
{
	unsigned long FsmStage;
	unsigned long CeEstEvm;
	int Constellation, Hierarchy;

	static const long SnrDbNumConst[DVBT_CONSTELLATION_NUM][DVBT_HIERARCHY_NUM] =
	{
		{122880,	122880,		122880,		122880,		},
		{146657,	146657,		156897,		171013,		},
		{167857,	167857,		173127,		181810,		},
	};

	long Var;
	MPI MpiCeEstEvm, MpiVar;



	// Get FSM stage, CE_EST_EVM, constellation, and hierarchy.
	if(GetRegBitsWithPage( DVBT_FSM_STAGE, &FsmStage) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	if(GetRegBitsWithPage( DVBT_CE_EST_EVM, &CeEstEvm) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	if(rtl2832_GetConstellation( &Constellation) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	if(rtl2832_GetHierarchy( &Hierarchy) != FUNCTION_SUCCESS)
		goto error_status_get_registers;



	// SNR dB formula
	// Original formula: SNR_dB = 10 * log10(Norm * pow(2, 11) / CeEstEvm)
	// Adjusted formula: SNR_dB = (SNR_DB_NUM_CONST - 10 * log2(CeEstEvm) * pow(2, SNR_FRAC_BIT_NUM)) / SNR_DB_DEN
	//                   SNR_DB_NUM_CONST = 10 * log2(Norm * pow(2, 11)) * pow(2, SNR_FRAC_BIT_NUM)
	//                   SNR_DB_DEN       = log2(10) * pow(2, SNR_FRAC_BIT_NUM)
	// Norm:
	//	                 None      Alpha=1   Alpha=2   Alpha=4
	//        4-QAM      2         2         2         2
	//       16-QAM      10        10        20        52
	//       64-QAM      42        42        60        108


	// If FSM stage < 10, set CE_EST_EVM with max value.
	if(FsmStage < 10)
		CeEstEvm = RTL2832_CE_EST_EVM_MAX_VALUE;


	// Calculate SNR dB numerator.
	MpiSetValue(&MpiCeEstEvm, CeEstEvm);

	MpiLog2(&MpiVar, MpiCeEstEvm, RTL2832_SNR_FRAC_BIT_NUM);

	MpiGetValue(MpiVar, &Var);

	*pSnrDbNum = SnrDbNumConst[Constellation][Hierarchy] - 10 * Var;

	
	// Set SNR dB denominator.
	*pSnrDbDen = RTL2832_SNR_DB_DEN;


	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_RF_AGC

*/
int rtl2832_GetRfAgc(	long *pRfAgc	)
{
	unsigned long Value;



	// Get RF_AGC_VAL to Value.
	if(GetRegBitsWithPage( DVBT_RF_AGC_VAL, &Value) != FUNCTION_SUCCESS)
		goto error_status_get_registers;


	// Convert Value to signed integer and store the signed integer to RfAgc.
	*pRfAgc = (int)BinToSignedInt(Value, RTL2832_RF_AGC_REG_BIT_NUM);


	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_IF_AGC

*/
int rtl2832_GetIfAgc(	long *pIfAgc	)
{
	unsigned long Value;
	// Get IF_AGC_VAL to Value.
	if(GetRegBitsWithPage( DVBT_IF_AGC_VAL, &Value) != FUNCTION_SUCCESS)
		goto error_status_get_registers;
	// Convert Value to signed integer and store the signed integer to IfAgc.
	*pIfAgc = (int)BinToSignedInt(Value, RTL2832_IF_AGC_REG_BIT_NUM);
	return FUNCTION_SUCCESS;
error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_DI_AGC

*/
int rtl2832_GetDiAgc(	unsigned char *pDiAgc	)
{
	unsigned long Value;



	// Get DAGC_VAL to DiAgc.
	if(GetRegBitsWithPage(DVBT_DAGC_VAL, &Value) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	*pDiAgc = (unsigned char)Value;


	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_TR_OFFSET_PPM

*/
int rtl2832_GetTrOffsetPpm(	long *pTrOffsetPpm	)
{
	unsigned long SfreqOffBinary;
	long SfreqOffInt;

	MPI MpiSfreqOffInt;
	MPI MpiConst, MpiVar;


	// Get SfreqOff binary value from SFREQ_OFF register bits.
	// Note: The function GetRegBitsWithPage() will set register page automatically.
	if(GetRegBitsWithPage(DVBT_SFREQ_OFF, &SfreqOffBinary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert SfreqOff binary value to signed integer.
	SfreqOffInt = BinToSignedInt(SfreqOffBinary, RTL2832_SFREQ_OFF_BIT_NUM);

	
	// Get TR offset in ppm.
	// Note: Original formula:   TrOffsetPpm = (SfreqOffInt * 1000000) / pow(2, 24)
	//       Adjusted formula:   TrOffsetPpm = (SfreqOffInt * 1000000) >> 24
	MpiSetValue(&MpiSfreqOffInt, SfreqOffInt);
	MpiSetValue(&MpiConst,       1000000);

	MpiMul(&MpiVar, MpiSfreqOffInt, MpiConst);
	MpiRightShift(&MpiVar, MpiVar, 24);

	MpiGetValue(MpiVar, pTrOffsetPpm);


	return FUNCTION_SUCCESS;


error_status_get_demod_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_CR_OFFSET_HZ

*/
int rtl2832_GetCrOffsetHz(	long *pCrOffsetHz	)
{
	int BandwidthMode;
	int FftMode;

	unsigned long CfreqOffBinary;
	long CfreqOffInt;

	long ConstWithBandwidthMode, ConstWithFftMode;

	MPI MpiCfreqOffInt;
	MPI MpiConstWithBandwidthMode, MpiConstWithFftMode;
	MPI MpiConst, MpiVar0, MpiVar1, MpiNone;



	// Get demod bandwidth mode.
	if(rtl2832_GetBandwidthMode(&BandwidthMode) != FUNCTION_SUCCESS)
		goto error_status_get_demod_bandwidth_mode;


	// Get demod FFT mode.
	if(rtl2832_GetFftMode(&FftMode) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;


	// Get CfreqOff binary value from CFREQ_OFF register bits.
	// Note: The function GetRegBitsWithPage() will set register page automatically.
	if(GetRegBitsWithPage( DVBT_CFREQ_OFF, &CfreqOffBinary) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;

	// Convert CfreqOff binary value to signed integer.
	CfreqOffInt = BinToSignedInt(CfreqOffBinary, RTL2832_CFREQ_OFF_BIT_NUM);


	// Determine constant value with bandwidth mode.
	switch(BandwidthMode)
	{
		default:
		case DVBT_BANDWIDTH_6MHZ:	ConstWithBandwidthMode = 48000000;		break;
		case DVBT_BANDWIDTH_7MHZ:	ConstWithBandwidthMode = 56000000;		break;
		case DVBT_BANDWIDTH_8MHZ:	ConstWithBandwidthMode = 64000000;		break;
	}


	// Determine constant value with FFT mode.
	switch(FftMode)
	{
		default:
		case DVBT_FFT_MODE_2K:		ConstWithFftMode = 2048;		break;
		case DVBT_FFT_MODE_8K:		ConstWithFftMode = 8192;		break;
	}


	// Get Cr offset in Hz.
	// Note: Original formula:   CrOffsetHz = (CfreqOffInt * ConstWithBandwidthMode) / (ConstWithFftMode * 7 * 128)
	//       Adjusted formula:   CrOffsetHz = (CfreqOffInt * ConstWithBandwidthMode) / ((ConstWithFftMode * 7) << 7)
	MpiSetValue(&MpiCfreqOffInt,            CfreqOffInt);
	MpiSetValue(&MpiConstWithBandwidthMode, ConstWithBandwidthMode);
	MpiSetValue(&MpiConstWithFftMode,       ConstWithFftMode);
	MpiSetValue(&MpiConst,                  7);

	MpiMul(&MpiVar0, MpiCfreqOffInt, MpiConstWithBandwidthMode);
	MpiMul(&MpiVar1, MpiConstWithFftMode, MpiConst);
	MpiLeftShift(&MpiVar1, MpiVar1, 7);
	MpiDiv(&MpiVar0, &MpiNone, MpiVar0, MpiVar1);

	MpiGetValue(MpiVar0, pCrOffsetHz);


	return FUNCTION_SUCCESS;


error_status_get_demod_registers:
error_status_get_demod_bandwidth_mode:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_CONSTELLATION

*/
int rtl2832_GetConstellation(	int *pConstellation	)
{
	unsigned long ReadingValue;


	// Get TPS constellation information from RX_CONSTEL.
	if(GetRegBitsWithPage( DVBT_RX_CONSTEL, &ReadingValue) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	switch(ReadingValue)
	{
		default:
		case 0:		*pConstellation = DVBT_CONSTELLATION_QPSK;			break;
		case 1:		*pConstellation = DVBT_CONSTELLATION_16QAM;			break;
		case 2:		*pConstellation = DVBT_CONSTELLATION_64QAM;			break;
	}


	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_HIERARCHY

*/
int rtl2832_GetHierarchy(	int *pHierarchy	)
{
	unsigned long ReadingValue;


	// Get TPS hierarchy infromation from RX_HIER.
	if(GetRegBitsWithPage( DVBT_RX_HIER, &ReadingValue) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	switch(ReadingValue)
	{
		default:
		case 0:		*pHierarchy = DVBT_HIERARCHY_NONE;				break;
		case 1:		*pHierarchy = DVBT_HIERARCHY_ALPHA_1;			break;
		case 2:		*pHierarchy = DVBT_HIERARCHY_ALPHA_2;			break;
		case 3:		*pHierarchy = DVBT_HIERARCHY_ALPHA_4;			break;
	}


	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_CODE_RATE_LP

*/
int rtl2832_GetCodeRateLp(	int *pCodeRateLp	)
{
	unsigned long ReadingValue;


	// Get TPS low-priority code rate infromation from RX_C_RATE_LP.
	if(GetRegBitsWithPage( DVBT_RX_C_RATE_LP, &ReadingValue) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	switch(ReadingValue)
	{
		default:
		case 0:		*pCodeRateLp = DVBT_CODE_RATE_1_OVER_2;			break;
		case 1:		*pCodeRateLp = DVBT_CODE_RATE_2_OVER_3;			break;
		case 2:		*pCodeRateLp = DVBT_CODE_RATE_3_OVER_4;			break;
		case 3:		*pCodeRateLp = DVBT_CODE_RATE_5_OVER_6;			break;
		case 4:		*pCodeRateLp = DVBT_CODE_RATE_7_OVER_8;			break;
	}


	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_CODE_RATE_HP

*/
int rtl2832_GetCodeRateHp(	int *pCodeRateHp	)
{
	unsigned long ReadingValue;


	// Get TPS high-priority code rate infromation from RX_C_RATE_HP.
	if(GetRegBitsWithPage( DVBT_RX_C_RATE_HP, &ReadingValue) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	switch(ReadingValue)
	{
		default:
		case 0:		*pCodeRateHp = DVBT_CODE_RATE_1_OVER_2;			break;
		case 1:		*pCodeRateHp = DVBT_CODE_RATE_2_OVER_3;			break;
		case 2:		*pCodeRateHp = DVBT_CODE_RATE_3_OVER_4;			break;
		case 3:		*pCodeRateHp = DVBT_CODE_RATE_5_OVER_6;			break;
		case 4:		*pCodeRateHp = DVBT_CODE_RATE_7_OVER_8;			break;
	}


	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_GUARD_INTERVAL

*/
int rtl2832_GetGuardInterval(	int *pGuardInterval	)
{
	unsigned long ReadingValue;


	// Get TPS guard interval infromation from GI_IDX.
	if(GetRegBitsWithPage( DVBT_GI_IDX, &ReadingValue) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	switch(ReadingValue)
	{
		default:
		case 0:		*pGuardInterval = DVBT_GUARD_INTERVAL_1_OVER_32;			break;
		case 1:		*pGuardInterval = DVBT_GUARD_INTERVAL_1_OVER_16;			break;
		case 2:		*pGuardInterval = DVBT_GUARD_INTERVAL_1_OVER_8;				break;
		case 3:		*pGuardInterval = DVBT_GUARD_INTERVAL_1_OVER_4;				break;
	}


	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_GET_FFT_MODE

*/
int rtl2832_GetFftMode(	int *pFftMode	)
{
	unsigned long ReadingValue;


	// Get TPS FFT mode infromation from FFT_MODE_IDX.
	if(GetRegBitsWithPage( DVBT_FFT_MODE_IDX, &ReadingValue) != FUNCTION_SUCCESS)
		goto error_status_get_registers;

	switch(ReadingValue)
	{
		default:
		case 0:		*pFftMode = DVBT_FFT_MODE_2K;			break;
		case 1:		*pFftMode = DVBT_FFT_MODE_8K;			break;
	}


	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_UPDATE_FUNCTION

*/
int rtl2832_UpdateFunction(	)
{
	// Execute Function 1 according to Function 1 enabling status
//	if(pExtra->IsFunc1Enabled == YES)
	{
		if(rtl2832_func1_Update() != FUNCTION_SUCCESS)
			goto error_status_execute_function;
	}


	return FUNCTION_SUCCESS;


error_status_execute_function:
	return FUNCTION_ERROR;
}





/**

@see   DVBT_DEMOD_FP_RESET_FUNCTION

*/
int rtl2832_ResetFunction(	)
{
	// Reset Function 1 settings according to Function 1 enabling status.
//	if(pExtra->IsFunc1Enabled == YES)
	{
		if(rtl2832_func1_Reset() != FUNCTION_SUCCESS)
			goto error_status_execute_function;
	}
	return FUNCTION_SUCCESS;
error_status_execute_function:
	return FUNCTION_ERROR;
}


/**

@brief   Reset Function 1 variables and registers.

One can use rtl2832_func1_Reset() to reset Function 1 variables and registers.


@param [in]   pDemod   The demod module pointer


@retval   FUNCTION_SUCCESS   Reset Function 1 variables and registers successfully.
@retval   FUNCTION_ERROR     Reset Function 1 variables and registers unsuccessfully.


@note
	-# Need to execute Function 1 reset function when change tuner RF frequency or demod parameters.
	-# Function 1 update flow also employs Function 1 reset function.

*/
int rtl2832_func1_Reset(	)
{

	Func1State               = RTL2832_FUNC1_STATE_NORMAL;
	Func1WaitTime            = 0;
	Func1GettingTime         = 0;
	Func1RsdBerEstSumNormal  = 0;
	Func1RsdBerEstSumConfig1 = 0;
	Func1RsdBerEstSumConfig2 = 0;
	Func1RsdBerEstSumConfig3 = 0;


	// Reset demod Function 1 registers.
    if(rtl2832_func1_ResetReg() != FUNCTION_SUCCESS)
		goto error_status_execute_function;


	return FUNCTION_SUCCESS;


error_status_execute_function:
	return FUNCTION_ERROR;
}





/**

@brief   Update demod registers with Function 1.

One can use rtl2832_func1_Update() to update demod registers with Function 1.


@param [in]   pDemod   The demod module pointer


@retval   FUNCTION_SUCCESS   Update demod registers with Function 1 successfully.
@retval   FUNCTION_ERROR     Update demod registers with Function 1 unsuccessfully.

*/

int rtl2832_func1_Update( 	)
{


	int Answer;
	int MinWeightedBerConfigMode;


	// Run FSM.
	switch(Func1State)
	{
		case RTL2832_FUNC1_STATE_NORMAL:

			// Ask if criterion is matched.
			if(rtl2832_func1_IsCriterionMatched( &Answer) != FUNCTION_SUCCESS)
				goto error_status_execute_function;

			if(Answer == YES)
			{
				// Accumulate RSD_BER_EST for normal case.
				if(rtl2832_func1_AccumulateRsdBerEst( &Func1RsdBerEstSumNormal) != FUNCTION_SUCCESS)
					goto error_status_execute_function;

				// Reset getting time counter.
				Func1GettingTime = 0;

				// Go to RTL2832_FUNC1_STATE_NORMAL_GET_BER state.
				Func1State = RTL2832_FUNC1_STATE_NORMAL_GET_BER;
			}

			break;


		case RTL2832_FUNC1_STATE_NORMAL_GET_BER:

			// Accumulate RSD_BER_EST for normal case.
			if(rtl2832_func1_AccumulateRsdBerEst( &Func1RsdBerEstSumNormal) != FUNCTION_SUCCESS)
				goto error_status_execute_function;

			// Use getting time counter to hold RTL2832_FUNC1_STATE_NORMAL_GET_BER state several times.
			Func1GettingTime += 1;

			if(Func1GettingTime >= Func1GettingTimeMax)
			{
				// Set common registers for configuration 1, 2, and 3 case.
				if(rtl2832_func1_SetCommonReg() != FUNCTION_SUCCESS)
					goto error_status_execute_function;

				// Set registers with FFT mode for configuration 1, 2, and 3 case.
				if(rtl2832_func1_SetRegWithFftMode( Func1FftBak) != FUNCTION_SUCCESS)
					goto error_status_execute_function;

				// Set registers for configuration 1 case.
				if(rtl2832_func1_SetRegWithConfigMode(RTL2832_FUNC1_CONFIG_1) != FUNCTION_SUCCESS)
					goto error_status_execute_function;

				// Reset demod by software reset.
				if(rtl2832_SoftwareReset() != FUNCTION_SUCCESS)
					goto error_status_execute_function;

				// Reset wait time counter.
				Func1WaitTime = 0;

				// Go to RTL2832_FUNC1_STATE_CONFIG_1_WAIT state.
				Func1State = RTL2832_FUNC1_STATE_CONFIG_1_WAIT;
			}

			break;


		case RTL2832_FUNC1_STATE_CONFIG_1_WAIT:

			// Use wait time counter to hold RTL2832_FUNC1_STATE_CONFIG_1_WAIT state several times.
			Func1WaitTime += 1;

			if(Func1WaitTime >= Func1WaitTimeMax)
			{
				// Accumulate RSD_BER_EST for configuration 1 case.
				if(rtl2832_func1_AccumulateRsdBerEst( &Func1RsdBerEstSumConfig1) != FUNCTION_SUCCESS)
					goto error_status_execute_function;

				// Reset getting time counter.
				Func1GettingTime = 0;

				// Go to RTL2832_FUNC1_STATE_CONFIG_1_GET_BER state.
				Func1State = RTL2832_FUNC1_STATE_CONFIG_1_GET_BER;
			}

			break;


		case RTL2832_FUNC1_STATE_CONFIG_1_GET_BER:

			// Accumulate RSD_BER_EST for configuration 1 case.
			if(rtl2832_func1_AccumulateRsdBerEst( &Func1RsdBerEstSumConfig1) != FUNCTION_SUCCESS)
				goto error_status_execute_function;

			// Use getting time counter to hold RTL2832_FUNC1_STATE_CONFIG_1_GET_BER state several times.
			Func1GettingTime += 1;

			if(Func1GettingTime >= Func1GettingTimeMax)
			{
				// Set registers for configuration 2 case.
				if(rtl2832_func1_SetRegWithConfigMode( RTL2832_FUNC1_CONFIG_2) != FUNCTION_SUCCESS)
					goto error_status_execute_function;

				// Reset demod by software reset.
				if(rtl2832_SoftwareReset() != FUNCTION_SUCCESS)
					goto error_status_execute_function;

				// Reset wait time counter.
				Func1WaitTime = 0;

				// Go to RTL2832_FUNC1_STATE_CONFIG_2_WAIT state.
				Func1State = RTL2832_FUNC1_STATE_CONFIG_2_WAIT;
			}

			break;


		case RTL2832_FUNC1_STATE_CONFIG_2_WAIT:

			// Use wait time counter to hold RTL2832_FUNC1_STATE_CONFIG_2_WAIT state several times.
			Func1WaitTime += 1;

			if(Func1WaitTime >= Func1WaitTimeMax)
			{
				// Accumulate RSD_BER_EST for configuration 2 case.
				if(rtl2832_func1_AccumulateRsdBerEst( &Func1RsdBerEstSumConfig2) != FUNCTION_SUCCESS)
					goto error_status_execute_function;

				// Reset getting time counter.
				Func1GettingTime = 0;

				// Go to RTL2832_FUNC1_STATE_CONFIG_2_GET_BER state.
				Func1State = RTL2832_FUNC1_STATE_CONFIG_2_GET_BER;
			}

			break;


		case RTL2832_FUNC1_STATE_CONFIG_2_GET_BER:

			// Accumulate RSD_BER_EST for configuration 2 case.
			if(rtl2832_func1_AccumulateRsdBerEst( &Func1RsdBerEstSumConfig2) != FUNCTION_SUCCESS)
				goto error_status_execute_function;

			// Use getting time counter to hold RTL2832_FUNC1_STATE_CONFIG_2_GET_BER state several times.
			Func1GettingTime += 1;

			if(Func1GettingTime >= Func1GettingTimeMax)
			{
				// Set registers for configuration 3 case.
				if(rtl2832_func1_SetRegWithConfigMode( RTL2832_FUNC1_CONFIG_3) != FUNCTION_SUCCESS)
					goto error_status_execute_function;

				// Reset demod by software reset.
				if(rtl2832_SoftwareReset() != FUNCTION_SUCCESS)
					goto error_status_execute_function;

				// Reset wait time counter.
				Func1WaitTime = 0;

				// Go to RTL2832_FUNC1_STATE_CONFIG_3_WAIT state.
				Func1State = RTL2832_FUNC1_STATE_CONFIG_3_WAIT;
			}

			break;


		case RTL2832_FUNC1_STATE_CONFIG_3_WAIT:

			// Use wait time counter to hold RTL2832_FUNC1_STATE_CONFIG_3_WAIT state several times.
			Func1WaitTime += 1;

			if(Func1WaitTime >= Func1WaitTimeMax)
			{
				// Accumulate RSD_BER_EST for configuration 3 case.
				if(rtl2832_func1_AccumulateRsdBerEst( &Func1RsdBerEstSumConfig3) != FUNCTION_SUCCESS)
					goto error_status_execute_function;

				// Reset getting time counter.
				Func1GettingTime = 0;

				// Go to RTL2832_FUNC1_STATE_CONFIG_3_GET_BER state.
				Func1State = RTL2832_FUNC1_STATE_CONFIG_3_GET_BER;
			}

			break;


		case RTL2832_FUNC1_STATE_CONFIG_3_GET_BER:

			// Accumulate RSD_BER_EST for configuration 3 case.
			if(rtl2832_func1_AccumulateRsdBerEst(&Func1RsdBerEstSumConfig3) != FUNCTION_SUCCESS)
				goto error_status_execute_function;

			// Use getting time counter to hold RTL2832_FUNC1_STATE_CONFIG_3_GET_BER state several times.
			Func1GettingTime += 1;

			if(Func1GettingTime >= Func1GettingTimeMax)
			{
				// Determine minimum-weighted-BER configuration mode.
				rtl2832_func1_GetMinWeightedBerConfigMode( &MinWeightedBerConfigMode);

				// Set registers with minimum-weighted-BER configuration mode.
				switch(MinWeightedBerConfigMode)
				{
					case RTL2832_FUNC1_CONFIG_NORMAL:

						// Reset registers for normal configuration.
						if(rtl2832_func1_ResetReg() != FUNCTION_SUCCESS)
							goto error_status_execute_function;

						break;


					case RTL2832_FUNC1_CONFIG_1:
					case RTL2832_FUNC1_CONFIG_2:
					case RTL2832_FUNC1_CONFIG_3:

						// Set registers for minimum-weighted-BER configuration.
						if(rtl2832_func1_SetRegWithConfigMode( MinWeightedBerConfigMode) != FUNCTION_SUCCESS)
							goto error_status_execute_function;

						break;


					default:

						// Get error configuration mode, reset registers.
						if(rtl2832_func1_ResetReg() != FUNCTION_SUCCESS)
							goto error_status_execute_function;

						break;
				}

				// Reset demod by software reset.
				if(rtl2832_SoftwareReset() != FUNCTION_SUCCESS)
					goto error_status_execute_function;

				// Reset wait time counter.
				Func1WaitTime = 0;

				// Go to RTL2832_FUNC1_STATE_DETERMINED_WAIT state.
				Func1State = RTL2832_FUNC1_STATE_DETERMINED_WAIT;
			}

			break;


		case RTL2832_FUNC1_STATE_DETERMINED_WAIT:

			// Use wait time counter to hold RTL2832_FUNC1_STATE_CONFIG_3_WAIT state several times.
			Func1WaitTime += 1;

			if(Func1WaitTime >= Func1WaitTimeMax)
			{
				// Go to RTL2832_FUNC1_STATE_DETERMINED state.
				Func1State = RTL2832_FUNC1_STATE_DETERMINED;
			}

			break;


		case RTL2832_FUNC1_STATE_DETERMINED:

			// Ask if criterion is matched.
			if(rtl2832_func1_IsCriterionMatched(&Answer) != FUNCTION_SUCCESS)
				goto error_status_execute_function;

			if(Answer == NO)
			{
				// Reset FSM.
				// Note: rtl2832_func1_Reset() will set FSM state with RTL2832_FUNC1_STATE_NORMAL.
				if(rtl2832_func1_Reset() != FUNCTION_SUCCESS)
					goto error_status_execute_function;
			}

			break;


		default:

			// Get error state, reset FSM.
			// Note: rtl2832_func1_Reset() will set FSM state with RTL2832_FUNC1_STATE_NORMAL.
			if(rtl2832_func1_Reset() != FUNCTION_SUCCESS)
				goto error_status_execute_function;

			break;
	}




	return FUNCTION_SUCCESS;


error_status_execute_function:
	return FUNCTION_ERROR;
}





/**

@brief   Ask if criterion is matched for Function 1.

One can use rtl2832_func1_IsCriterionMatched() to ask if criterion is matched for Function 1.


@param [in]    pDemod    The demod module pointer
@param [out]   pAnswer   Pointer to an allocated memory for storing answer


@retval   FUNCTION_SUCCESS   Ask if criterion is matched for Function 1 successfully.
@retval   FUNCTION_ERROR     Ask if criterion is matched for Function 1 unsuccessfully.

*/
int rtl2832_func1_IsCriterionMatched(	int *pAnswer	)
{
	unsigned long FsmStage;

	int Qam;
	int Hier;
	int LpCr;
	int HpCr;
	int Gi;
	int Fft;

	unsigned long Reg0, Reg1;

	int BandwidthMode;

	// Get FSM_STAGE.
    if(GetRegBitsWithPage( DVBT_FSM_STAGE, &FsmStage) != FUNCTION_SUCCESS)
		goto error_status_get_registers;


	// Get QAM.
	if(rtl2832_GetConstellation( &Qam) != FUNCTION_SUCCESS)
		goto error_status_execute_function;

	// Get hierarchy.
	if(rtl2832_GetHierarchy(&Hier) != FUNCTION_SUCCESS)
		goto error_status_execute_function;

	// Get low-priority code rate.
	if(rtl2832_GetCodeRateLp( &LpCr) != FUNCTION_SUCCESS)
		goto error_status_execute_function;

	// Get high-priority code rate.
	if(rtl2832_GetCodeRateHp( &HpCr) != FUNCTION_SUCCESS)
		goto error_status_execute_function;

	// Get guard interval.
	if(rtl2832_GetGuardInterval( &Gi) != FUNCTION_SUCCESS)
		goto error_status_execute_function;

	// Get FFT mode.
	if(rtl2832_GetFftMode( &Fft) != FUNCTION_SUCCESS)
		goto error_status_execute_function;


	// Get REG_0 and REG_1.
	if(SetRegPage( 0x3) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(GetRegMaskBits( 0x22, 0, 0, &Reg0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(GetRegMaskBits( 0x1a, 15, 3, &Reg1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;


	// Get bandwidth mode.
	if(rtl2832_GetBandwidthMode(&BandwidthMode) != FUNCTION_SUCCESS)
		goto error_status_execute_function;


	// Determine criterion answer.
	*pAnswer = 
		(FsmStage == 11) && 

		(Qam  == Func1QamBak) &&
		(Hier == Func1HierBak) &&
		(LpCr == Func1LpCrBak) &&
		(HpCr == Func1HpCrBak) &&
		(Gi   == Func1GiBak) &&
		(Fft  == Func1FftBak) &&

		(Reg0 == 0x1) &&

		((BandwidthMode == DVBT_BANDWIDTH_8MHZ) &&
		 ( ((Fft == DVBT_FFT_MODE_2K) && (Reg1 > 1424) && (Reg1 < 1440)) ||
		   ((Fft == DVBT_FFT_MODE_8K) && (Reg1 > 5696) && (Reg1 < 5760))    ) );


	// Backup TPS information.
	Func1QamBak  = Qam;
	Func1HierBak = Hier;
	Func1LpCrBak = LpCr;
	Func1HpCrBak = HpCr;
	Func1GiBak   = Gi;
	Func1FftBak  = Fft;


	return FUNCTION_SUCCESS;


error_status_set_registers:
error_status_execute_function:
error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@brief   Accumulate RSD_BER_EST value for Function 1.

One can use rtl2832_func1_AccumulateRsdBerEst() to accumulate RSD_BER_EST for Function 1.


@param [in]   pDemod               The demod module pointer
@param [in]   pAccumulativeValue   Accumulative RSD_BER_EST value


@retval   FUNCTION_SUCCESS   Accumulate RSD_BER_EST for Function 1 successfully.
@retval   FUNCTION_ERROR     Accumulate RSD_BER_EST for Function 1 unsuccessfully.

*/
int rtl2832_func1_AccumulateRsdBerEst(	unsigned long *pAccumulativeValue	)
{
	int i;
	unsigned long RsdBerEst;

	// Get RSD_BER_EST with assigned times.
	for(i = 0; i < Func1GettingNumEachTime; i++)
	{
		// Get RSD_BER_EST.
		if(GetRegBitsWithPage(DVBT_RSD_BER_EST, &RsdBerEst) != FUNCTION_SUCCESS)
			goto error_status_get_registers;

		// Accumulate RSD_BER_EST to accumulative value.
		*pAccumulativeValue += RsdBerEst;
	}


	return FUNCTION_SUCCESS;


error_status_get_registers:
	return FUNCTION_ERROR;
}





/**

@brief   Reset registers for Function 1.

One can use rtl2832_func1_ResetReg() to reset registers for Function 1.


@param [in]   pDemod   The demod module pointer


@retval   FUNCTION_SUCCESS   Reset registers for Function 1 successfully.
@retval   FUNCTION_ERROR     Reset registers for Function 1 unsuccessfully.

*/
int rtl2832_func1_ResetReg()
{
	// Reset Function 1 registers.
    if(SetRegPage( 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0x65, 2, 0, 0x7) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0x68, 5, 4, 0x3) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0x5b, 2, 0, 0x5) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0x5b, 5, 3, 0x5) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0x5c, 2, 0, 0x5) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0x5c, 5, 3, 0x5) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0xd0, 3, 2, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0xd1, 14, 0, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0xd3, 14, 0, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0xd5, 14, 0, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegPage( 0x2) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0x1, 0, 0, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0xb4, 7, 6, 0x3) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0xd2, 1, 1, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0xb5, 7, 7, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;


	return FUNCTION_SUCCESS;


error_status_set_registers:
	return FUNCTION_ERROR;
}





/**

@brief   Set common registers for Function 1.

One can use rtl2832_func1_SetCommonReg() to set common registers for Function 1.


@param [in]   pDemod   The demod module pointer


@retval   FUNCTION_SUCCESS   Set common registers for Function 1 successfully.
@retval   FUNCTION_ERROR     Set common registers for Function 1 unsuccessfully.

*/
int rtl2832_func1_SetCommonReg(	)
{
	// Set common registers for Function 1.
    if(SetRegPage( 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0x65, 2, 0, 0x5) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0x68, 5, 4, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegPage( 0x2) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0xd2, 1, 1, 0x1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits( 0xb5, 7, 7, 0x0) != FUNCTION_SUCCESS)
		goto error_status_set_registers;


	return FUNCTION_SUCCESS;


error_status_set_registers:
	return FUNCTION_ERROR;
}





/**

@brief   Set registers with FFT mode for Function 1.

One can use rtl2832_func1_SetRegWithConfigMode() to set registers with FFT mode for Function 1.


@param [in]   pDemod    The demod module pointer
@param [in]   FftMode   FFT mode for setting


@retval   FUNCTION_SUCCESS   Set registers with FFT mode for Function 1 successfully.
@retval   FUNCTION_ERROR     Set registers with FFT mode for Function 1 unsuccessfully.

*/
int rtl2832_func1_SetRegWithFftMode(	int FftMode	)
{
	typedef struct
	{
		unsigned long Reg0[DVBT_FFT_MODE_NUM];
		unsigned long Reg1[DVBT_FFT_MODE_NUM];
	}
	FFT_REF_ENTRY;



	static const FFT_REF_ENTRY FftRefTable =
	{
		// 2K mode,   8K mode
		{0x0,         0x1    },
		{0x3,         0x0    },
	};



	// Set registers with FFT mode for Function 1.
    if(SetRegPage(0x2) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits(0x1, 0, 0, FftRefTable.Reg0[FftMode]) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits(0xb4, 7, 6, FftRefTable.Reg1[FftMode]) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

	return FUNCTION_SUCCESS;

error_status_set_registers:
	return FUNCTION_ERROR;
}





/**

@brief   Set registers with configuration mode for Function 1.

One can use rtl2832_func1_SetRegWithConfigMode() to set registers with configuration mode for Function 1.


@param [in]   pDemod       The demod module pointer
@param [in]   ConfigMode   Configuration mode for setting


@retval   FUNCTION_SUCCESS   Set registers with configuration mode for Function 1 successfully.
@retval   FUNCTION_ERROR     Set registers with configuration mode for Function 1 unsuccessfully.


@note
	-# This function can not set RTL2832_FUNC1_CONFIG_NORMAL configuration mode.

*/
int rtl2832_func1_SetRegWithConfigMode(	int ConfigMode	)
{
	typedef struct
	{
		unsigned long Reg0[RTL2832_FUNC1_CONFIG_MODE_NUM];
		unsigned long Reg1[RTL2832_FUNC1_CONFIG_MODE_NUM];
		unsigned long Reg2[RTL2832_FUNC1_CONFIG_MODE_NUM];
		unsigned long Reg3[RTL2832_FUNC1_CONFIG_MODE_NUM];
		unsigned long Reg4[RTL2832_FUNC1_CONFIG_MODE_NUM];

		unsigned long Reg5Ref[RTL2832_FUNC1_CONFIG_MODE_NUM];
		unsigned long Reg6Ref[RTL2832_FUNC1_CONFIG_MODE_NUM];
		unsigned long Reg7Ref[RTL2832_FUNC1_CONFIG_MODE_NUM];
	}
	CONFIG_REF_ENTRY;

	static const CONFIG_REF_ENTRY ConfigRefTable =
	{
		// Config 1,   Config 2,   Config 3
		{0x5,          0x4,        0x5     },
		{0x5,          0x4,        0x7     },
		{0x5,          0x4,        0x7     },
		{0x7,          0x6,        0x5     },
		{0x3,          0x3,        0x2     },

		{4437,         4437,       4325    },
		{6000,         5500,       6500    },
		{6552,         5800,       5850    },
	};

	int BandwidthMode;

	static const unsigned long Const[DVBT_BANDWIDTH_MODE_NUM] =
	{
		// 6Mhz, 7Mhz, 8Mhz
		48,      56,   64,
	};

	unsigned long Reg5, Reg6, Reg7;

	// Get bandwidth mode.
	if(rtl2832_GetBandwidthMode(&BandwidthMode) != FUNCTION_SUCCESS)
		goto error_status_execute_function;

	// Calculate REG_5, REG_6, and REG_7 with bandwidth mode and configuration mode.
	Reg5 = (ConfigRefTable.Reg5Ref[ConfigMode] * 7 * 2048 * 8) / (1000 * Const[BandwidthMode]);
	Reg6 = (ConfigRefTable.Reg6Ref[ConfigMode] * 7 * 2048 * 8) / (1000 * Const[BandwidthMode]);
	Reg7 = (ConfigRefTable.Reg7Ref[ConfigMode] * 7 * 2048 * 8) / (1000 * Const[BandwidthMode]);


	// Set registers with bandwidth mode and configuration mode.
    if(SetRegPage(0x1) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits(0x5b, 2, 0, ConfigRefTable.Reg0[ConfigMode]) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits(0x5b, 5, 3, ConfigRefTable.Reg1[ConfigMode]) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits(0x5c, 2, 0, ConfigRefTable.Reg2[ConfigMode]) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits(0x5c, 5, 3, ConfigRefTable.Reg3[ConfigMode]) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits(0xd0, 3, 2, ConfigRefTable.Reg4[ConfigMode]) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits(0xd1, 14, 0, Reg5) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits(0xd3, 14, 0, Reg6) != FUNCTION_SUCCESS)
		goto error_status_set_registers;

    if(SetRegMaskBits(0xd5, 14, 0, Reg7) != FUNCTION_SUCCESS)
		goto error_status_set_registers;


	return FUNCTION_SUCCESS;


error_status_set_registers:
error_status_execute_function:
	return FUNCTION_ERROR;
}




void rtl2832_func1_GetMinWeightedBerConfigMode(	int *pConfigMode	)
{
	unsigned long WeightedBerNormal;
	unsigned long WeightedBerConfig1;
	unsigned long WeightedBerConfig2;
	unsigned long WeightedBerConfig3;

	// Calculate weighted BER for all configuration mode
	WeightedBerNormal  = Func1RsdBerEstSumNormal * 2;
	WeightedBerConfig1 = Func1RsdBerEstSumConfig1;
	WeightedBerConfig2 = Func1RsdBerEstSumConfig2;
	WeightedBerConfig3 = Func1RsdBerEstSumConfig3;


	// Determine minimum-weighted-BER configuration mode.
	if(WeightedBerNormal <= WeightedBerConfig1 &&
		WeightedBerNormal <= WeightedBerConfig2 &&
		WeightedBerNormal <= WeightedBerConfig3)
	{
		*pConfigMode = RTL2832_FUNC1_CONFIG_NORMAL;
	}
	else if(WeightedBerConfig1 <= WeightedBerNormal &&
		WeightedBerConfig1 <= WeightedBerConfig2 &&
		WeightedBerConfig1 <= WeightedBerConfig3)
	{
		*pConfigMode = RTL2832_FUNC1_CONFIG_1;
	}
	else if(WeightedBerConfig2 <= WeightedBerNormal &&
		WeightedBerConfig2 <= WeightedBerConfig1 &&
		WeightedBerConfig2 <= WeightedBerConfig3)
	{
		*pConfigMode = RTL2832_FUNC1_CONFIG_2;
	}
	else if(WeightedBerConfig3 <= WeightedBerNormal &&
		WeightedBerConfig3 <= WeightedBerConfig1 &&
		WeightedBerConfig3 <= WeightedBerConfig2)
	{
		*pConfigMode = RTL2832_FUNC1_CONFIG_3;
	}

	return;
}











