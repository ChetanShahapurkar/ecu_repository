/**
 *  @file
 *  @brief     This file contains the definition of commonly used Macro and Enumerators
 *
 *  
 *
 *  @copyright  Copyright 2007-. Whirlpool Corporation. All rights reserved - CONFIDENTIAL
 */

//=====================================================================================================================
//-------------------------------------- PUBLIC (Extern Variables, Constants & Defines) -------------------------------
//=====================================================================================================================
// ============================================================
// --------------- STANDARD SYSTEM DEFINITIONS ----------------
// ------------------------------------------------------------
typedef enum _BOOL_TYPE
{
    FALSE           = 0,
    TRUE            = 1
} BOOL_TYPE;

typedef BOOL_TYPE T_BOOL;

typedef enum _ON_OFF_TYPE
{
    OFF             = 0,
    ON              = 1
} ON_OFF_TYPE;


typedef enum _COMPLETE_TYPE
{
    INCOMPLETE      = 0,
    COMPLETE        = 1
} COMPLETE_TYPE;

typedef enum _ACTIVE_TYPE
{
    INACTIVE        = 0,
    ACTIVE          = 1
} ACTIVE_TYPE;


#ifndef ENABLED
	#define ENABLED   		(1)
#endif

#ifndef DISABLED
	#define DISABLED  		(0)
#endif

#ifndef NULL
	#define NULL 			(void *)0
#endif

#ifndef UNAVAILABLE
	#define UNAVAILABLE 	(10)
#endif

#ifndef AVAILABLE
	#define AVAILABLE 		(11)
#endif

//---------------------------------------------------------------------------------------------------------------------
//------------------- --------------------------BYTE AND WORD ACCESS --------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------

#define LOWORD(mem)         (unsigned short)((mem) & 0xFFFFU)   ///< This macro is for extracting lower word from a double word type of data
#define LOBYTE(mem)         (unsigned char)((mem) & 0xFFU)      ///< This macro is for extracting lower byte from the word type of data
#define HIWORD(mem)         (unsigned short)(((unsigned long)(mem) >> 16U ) & 0xFFFFU)  ///< This macro is for extracting higher word from a double word type of data
#define HIBYTE(mem)         (unsigned char)(((unsigned short)(mem) >> 8U) & 0xFFU)      ///<This macro is for extracting higher byte from a word type of data
//---------------------------------------------------------------------------------------------------------------------
//--------------------------------------------- BIT MANIPULATION ------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------

#define BIT_TEST(mem,bit)               ((mem)&(1UL<<(bit)))    ///< This macro is used to perform test on bit of the data
#define BIT_SET(mem,bit)                ((mem)|=(1UL<<(bit)))   ///< This macro is used to set the bit of the data
#define BIT_CLR(mem,bit)                ((mem)&=~(1UL<<(bit)))  ///< This macro is used to clear the bit of the data
#define BIT_TOGGLE(mem,bit)             ((mem)^=(1UL<<(bit)))   ///< This macro is used to toggle the bit of the data
#define BIT_STORAGE_SIZE(num_of_bits)   ((size_t)(((num_of_bits) + 7U) / 8U)) ///< this macro returns the number of bytes needed to store the amount bits requested.

#define BYTES_REQUIRED(bits)            ((unsigned char)(((bits)+7)>>3))///< Macro to convert bits to bytes with minimal calculation (in case it must be at run time).

//---------------------------------------------------------------------------------------------------------------------
//--------------------------------------------- DATA ALIAS ------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
typedef unsigned char       uint8;
typedef unsigned short int  uint16;
typedef unsigned long int   uint32;

#ifndef __CSMC__
    typedef unsigned long long  uint64;
#endif

typedef signed char         sint8;
typedef signed short int    sint16;
typedef signed long int     sint32;

#ifndef __CSMC__
    typedef signed long long    sint64;
#endif

typedef float               float32;
typedef double              float64;
