
#ifndef __DTMF_TYPES_H__
#define __DTMF_TYPES_H__
#include <string.h>
#include <pthread.h>
typedef 	unsigned char 		U8bit;
typedef 	unsigned short		U16bit;
typedef		unsigned int		U32bit;
typedef		unsigned long		U64bit;

typedef 	char			    S8bit;
typedef		short				S16bit;
typedef		int					S32bit;

typedef		void *				Pvoid;

typedef 	U8bit				Sdf_ty_u8bit;
typedef 	U16bit				Sdf_ty_u16bit;
typedef	 	U32bit				Sdf_ty_u32bit;
typedef	 	U64bit				Sdf_ty_u64bit;


typedef 	S8bit				Sdf_ty_s8bit;
typedef		S16bit				Sdf_ty_s16bit;
typedef		S32bit				Sdf_ty_s32bit;
typedef		long				Sdf_ty_s64bit;

typedef     void            	Sdf_ty_void; 
typedef		Pvoid				Sdf_ty_pvoid;
//Added  by Quanjie.Deng s  2015.6.5
//type of  send  to rtpproxy
typedef  struct T_ToRtp
{
	int  src_port;
	int  dtmf_sinal;
	T_ToRtp(){
		src_port 	= 0;
		dtmf_sinal 	= -1;
		}
	~T_ToRtp(){
		}
	
	
}To_Rtp;
//Added  by Quanjie.Deeg e  2105.6.5

//--------------------------------------
//DESCRIPTION:
//This file include all the  protocol head struct 
//include UDP and  RTP and some others
//--------------------------------------

//-------------------------------------
//struct UDP head 
//-------------------------------------
typedef struct UserDataProtocol
{
	unsigned short srcPort;
	unsigned short dstPort;
	unsigned short length;
	unsigned short checksum;	
}UserDataProtocol_t;
//-------------------------------------
//struct rtp head 
//-------------------------------------
typedef struct RtpHeader
{
	unsigned char version;
	unsigned char Marker_palodtype;
	unsigned short squence_number;
	unsigned int timestamp;
	unsigned int souce_indentfer;
}RtpHeader_t;

//-----------------------------------
//RFC2833 Payload DATA struct
//-----------------------------------

typedef struct RFC2833Payload
{
	unsigned char  EventType;
	unsigned char  E_R_v_en;//this argument is include all the 'end' 'res' 'volme'
	unsigned short Duration;
}RFC2833Payload_t;

//保存应用层和DTNF模块之间的数据
typedef struct t_app_dtmf
{
	char * rcv_msg;
	long   rcv_port;
}app_dtmf_t;
#endif


