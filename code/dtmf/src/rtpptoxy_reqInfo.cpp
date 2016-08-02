#include "rtpptoxy_reqInfo.h"
#include "Dtmf_types.h"
#include <arpa/inet.h>
#include <stdio.h>
PxyReqRecordInfo * PxyReqRecordInfo::instance	= 	NULL;

//Added  by Quanjie.Deng s 2015.6.5
//----------------------------------------
//FUNC:show_RTP_func
//DESCRIPTION:
//this fun whill help show the  RTP page msg.
//----------------------------------------
void PxyReqRecordInfo::show_RTP_func(RtpHeader_t rtp)
{
 	printf("Information  RTP\n");
	printf("V:%d\n",ntohs(rtp.version));
	printf("Marker:(%s)\n",((int)(rtp.Marker_palodtype&0x80))?"ture":"fale");
	printf("paload-type:%d\n",rtp.Marker_palodtype);
	printf("squence_number:%u\n",ntohs(rtp.squence_number));
	printf("timestamp:%d\n",ntohl(rtp.timestamp));
	printf("souce_indentfer:%x\n",ntohl(rtp.souce_indentfer));
	printf("----------------------------------------\n");
}
void PxyReqRecordInfo::show_RTP_func_log(RtpHeader_t rtp)
{
 	clm_log(CLM_DEBUG,"Information  RTP\n");
	clm_log(CLM_DEBUG,"V:%d\n",ntohs(rtp.version));
	clm_log(CLM_DEBUG,"Marker:(%s)\n",((int)(rtp.Marker_palodtype&0x80))?"ture":"fale");
	clm_log(CLM_DEBUG,"paload-type:%d\n",rtp.Marker_palodtype);
	//clm_log(CLM_DEBUG,"paload-type:%x\n",rtp.Marker_palodtype&0x7f);
	clm_log(CLM_DEBUG,"squence_number:%u\n",ntohs(rtp.squence_number));
	clm_log(CLM_DEBUG,"timestamp:%ld\n",ntohl(rtp.timestamp));
	clm_log(CLM_DEBUG,"souce_indentfer:%x\n",ntohl(rtp.souce_indentfer));
	clm_log(CLM_DEBUG,"----------------------------------------\n");
}

//-----------------------------------------
//FUNC:show_2833_func
//DESCRIPTION:this func whill help show the  msg 
//of the  2888-msg body.
//-----------------------------------------
void PxyReqRecordInfo::show_2833_func(RFC2833Payload_t rfc2833)
{
	printf("*----2833payload-------*\n");
	printf("Event:%u\n", rfc2833.EventType);
	printf("END:%d\n", (rfc2833.E_R_v_en&0x80)>>7);
	printf("Res:%d\n", (rfc2833.E_R_v_en&0x40)>>6);
	printf("Vloune:%u\n", rfc2833.E_R_v_en&0xF);
	printf("Duration:%x\n", ntohl(rfc2833.Duration));
	printf("*--------------------------*\n\n");
}
void PxyReqRecordInfo::show_2833_func_log(RFC2833Payload_t rfc2833)
{
	clm_log(CLM_DEBUG,"*----2833payload-------*\n");
	clm_log(CLM_DEBUG,"Event:%u\n", rfc2833.EventType);
	clm_log(CLM_DEBUG,"END:%d\n", (rfc2833.E_R_v_en&0x80)>>7);
	clm_log(CLM_DEBUG,"Res:%d\n", (rfc2833.E_R_v_en&0x40)>>6);
	clm_log(CLM_DEBUG,"Vloune:%u\n", rfc2833.E_R_v_en&0xF);
	clm_log(CLM_DEBUG,"Duration:%x\n", ntohl(rfc2833.Duration));
	clm_log(CLM_DEBUG,"*--------------------------*\n\n");
}


//Added  by Quanjie.Degn  s  2015.07.17
int  PxyReqRecordInfo::handle_msgUDP(char * rcv_udp, int *dtmf_signal,long* timestamp)
{
    
	if(rcv_udp == NULL)
	{
		clm_log(CLM_ERROR,"func handle_msgUDP the  recv msg is null!");
		return 0;
	}

   	RtpHeader_t head_rtp;
	RFC2833Payload_t   payload_2833;
	
	bzero(&head_rtp, sizeof(head_rtp));
	bzero(&payload_2833, sizeof(payload_2833));
	char * rcv_tmp = rcv_udp;
	
	//1.cut RTP head.
	memcpy(&head_rtp, rcv_tmp, sizeof(RtpHeader_t));
	rcv_tmp+=sizeof(head_rtp);
	//2.判断是否为RFC2833
#ifdef DEBUG_LOG_RTP
	this->show_RTP_func_log(head_rtp);
#endif
	int  tmp_type = (unsigned int)(head_rtp.Marker_palodtype);
    tmp_type = tmp_type&127;//127==0x7f
    clm_log(CLM_DEBUG,"recv_content_type:[%d]\n",tmp_type);
	if(tmp_type == 101)
	{
		memcpy(&payload_2833, rcv_tmp, sizeof(RFC2833Payload_t));
#ifdef DEBUG_LOG_2833
        this->show_2833_func_log(payload_2833);
#endif
        //判断是否为事物结束
        if (((payload_2833.E_R_v_en&0x80)>>7) == 1)
        {
            clm_log(CLM_INFO,"new DTNF:[%d]\n",(unsigned int)payload_2833.EventType);
            *dtmf_signal = (unsigned int)payload_2833.EventType;
	        *timestamp = ntohl(head_rtp.timestamp);
            delete []rcv_udp;
            return 1;
        }
        else
        {
            ;
        }
		
	}
    delete []rcv_udp;
	return  0;
}
//Added  by Quanjei.Deng  e  2015.07.17








