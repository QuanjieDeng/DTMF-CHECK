#ifndef _SDF_COMMON_H_
#define _SDF_COMMON_H_

#include "pthread.h"
#include <iostream>

typedef     unsigned char       Sdf_ty_U8bit;
typedef     unsigned short	    Sdf_ty_U16bit;
typedef	    unsigned int	    Sdf_ty_U32bit;
typedef	    unsigned long	    Sdf_ty_U64bit;
typedef     char                Sdf_ty_S8bit;
typedef	    short               Sdf_ty_S16bit;
typedef	    int	                Sdf_ty_S32bit;
typedef	    void *              Sdf_ty_Pvoid;

typedef 	pthread_t           Sdf_ty_threadT;
typedef 	pthread_mutex_t     Sdf_ty_mutexT;
typedef 	pthread_cond_t 	    Sdf_ty_condT;
typedef 	pthread_attr_t      Sdf_ty_threadAttrT;
typedef 	pthread_mutexattr_t Sdf_ty_mutexAttrT;
typedef 	pthread_condattr_t  Sdf_ty_condAttrT;


#define 	Sdf_co_u32Max		   4294967295ul
#define 	Sdf_co_s32MaxStr	   "2147483647"
#define 	Sdf_co_u32MaxStr       "4294967295"
#define     Sdf_co_u16Max          65535
#define     Sdf_co_u32MaxLen       10
#define     Sdf_co_u16MaxStr       "65535"
#define     Sdf_co_u16MaxLen       5
#define     Sdf_co_maxLogFileSize  1048576
#define		Sdf_co_null		       0

#define 	SDF_IN                 const
#define 	SDF_OUT
#define 	SDF_INOUT
#define     UDP_RECV_LEN           2018


typedef enum
{
	/* ------------------------------------------------------------------
	 DESCRIPTION:	Enum defining the return values.
	 
	 Sdf_co_success	-	Return value indicating success.
	 Sdf_co_fail	-	Return value indicating failure.
	 -----------------------------------------------------------------*/

	Sdf_co_fail    = -1,
	Sdf_co_success =  0
} Sdf_ty_retVal;

typedef enum
{
	/* ------------------------------------------------------------------------
	 DESCRIPTION:	Enum defining the Boolean values.
	 
	 Sdf_co_true	- TRUE boolean.
	 Sdf_co_false - FALSE boolean.
	-----------------------------------------------------------------------*/
	Sdf_co_false   = 0,
	Sdf_co_true	   = 1
} Sdf_ty_bool;

typedef enum
{
	Sdf_en_errorDuplicateData = 0,
		
}Sdf_ty_error;

typedef enum
{
       Cdr_FromAppMsg = 0,
	DTMF_FromProxyMsg = 1,
}cdr_messageId;

 typedef struct dtmf_st_msgFormat
{
	Sdf_ty_U32bit		 msgType; 
	void 				*pMsgData;
	dtmf_st_msgFormat() 
	{
	        pMsgData = Sdf_co_null;
	        msgType = 0;
	 }
	~dtmf_st_msgFormat()
	{
	}	 
}dtmf_msgFormat;
#endif

