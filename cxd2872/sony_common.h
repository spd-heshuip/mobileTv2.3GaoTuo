/*------------------------------------------------------------------------------
  Copyright 2009-2013 Sony Corporation

  Last Updated  : 2013/12/20
  File Revision : 1.0.3.0
------------------------------------------------------------------------------*/

#ifndef SONY_COMMON_H
#define SONY_COMMON_H

/* Type definitions. */
/* <PORTING> Please comment out if conflicted */
/*
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;
*/
#ifndef uint8_t
#define uint8_t unsigned char
#endif
#ifndef uint16_t
#define uint16_t unsigned int
#endif
#ifndef uint32_t
#define uint32_t unsigned long
#endif
#ifndef int8_t
#define int8_t  char
#endif
#ifndef int16_t
#define int16_t signed int
#endif
#ifndef int32_t
#define int32_t signed long
#endif

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

/* <PORTING> Sleep function define (Please modify as you like) */
//#define _WINDOWS
#ifdef _WINDOWS
//#include <windows.h>
#define SONY_SLEEP(n) //Sleep(n*50)
#endif

#ifndef SONY_SLEEP

#define SONY_SLEEP(n) WaitLockTime(n*35)//is not defined. This macro must be ported to your platform.
#endif


/* <PORTING> Floating point enable */
/* #define SONY_FP_ENABLE */

/* <PORTING> Trace function enable */
/* #define SONY_TRACE_ENABLE */
/* <PORTING> Enable if I2C related function trace is necessary */
/* #define SONY_TRACE_I2C_ENABLE */

/* <PORTING> Macro to specify unused argument and suppress compiler warnings. */
#define SONY_ARG_UNUSED(arg) ((arg) = (arg))

/*------------------------------------------------------------------------------
  Enums
------------------------------------------------------------------------------*/
/* Return codes */
typedef enum {
    SONY_RESULT_OK,
    SONY_RESULT_ERROR_ARG,      /* Invalid argument (maybe software bug) */
    SONY_RESULT_ERROR_I2C,      /* I2C communication error */
    SONY_RESULT_ERROR_SW_STATE, /* Invalid software state */
    SONY_RESULT_ERROR_HW_STATE, /* Invalid hardware state */
    SONY_RESULT_ERROR_TIMEOUT,  /* Timeout occured */
    SONY_RESULT_ERROR_UNLOCK,   /* Failed to lock */
    SONY_RESULT_ERROR_RANGE,    /* Out of range */
    SONY_RESULT_ERROR_NOSUPPORT,/* Not supported for current device */
    SONY_RESULT_ERROR_CANCEL,   /* The operation is canceled */
    SONY_RESULT_ERROR_OTHER
} sony_result_t;

/*------------------------------------------------------------------------------
  Common functions
------------------------------------------------------------------------------*/
int32_t sony_Convert2SComplement(uint32_t value, uint32_t bitlen);
uint32_t sony_BitSplitFromByteArray(uint8_t *pArray, uint32_t startBit, uint32_t bitNum);

/*------------------------------------------------------------------------------
  Trace
------------------------------------------------------------------------------*/
#ifdef SONY_TRACE_ENABLE
/* <PORTING> This is only a sample of trace macro. Please modify is necessary. */
/* In sample application, these functions are implemented in main.c */
void sony_trace_log_enter(const char* funcname, const char* filename, unsigned int linenum);
void sony_trace_log_return(sony_result_t result, const char* filename, unsigned int linenum);
#define SONY_TRACE_ENTER(func) sony_trace_log_enter((func), __FILE__, __LINE__)
#define SONY_TRACE_RETURN(result) do{sony_trace_log_return((result), __FILE__, __LINE__); return (result);}while(0)
#else /* SONY_TRACE_ENABLE */
#define SONY_TRACE_ENTER(func)
#define SONY_TRACE_RETURN(result) return(result)
#define SONY_TRACE_I2C_ENTER(func)
#define SONY_TRACE_I2C_RETURN(result) return(result)
#endif /* SONY_TRACE_ENABLE */


#ifdef SONY_TRACE_I2C_ENABLE
/* <PORTING> This is only a sample of trace macro. Please modify is necessary. */
void sony_trace_i2c_log_enter(const char* funcname, const char* filename, unsigned int linenum);
void sony_trace_i2c_log_return(sony_result_t result, const char* filename, unsigned int linenum);
#define SONY_TRACE_I2C_ENTER(func) sony_trace_i2c_log_enter((func), __FILE__, __LINE__)
#define SONY_TRACE_I2C_RETURN(result) do{sony_trace_i2c_log_return((result), __FILE__, __LINE__); return (result);}while(0)
#else /* SONY_TRACE_I2C_ENABLE */
#define SONY_TRACE_I2C_ENTER(func)
#define SONY_TRACE_I2C_RETURN(result) return(result)
#endif /* SONY_TRACE_I2C_ENABLE */

#endif /* SONY_COMMON_H */
