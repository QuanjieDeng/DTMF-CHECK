#include "Dtmf_MessageStack.h"
#include <stdlib.h>
#include <time.h>
#include "Dtmf_types.h"
#include "rtpptoxy_reqInfo.h"

Sdf_ty_retVal DTMF_MessageStack::handleMessage(void * pmsg,DatagramSocket * m_myServer)
{
	(void)m_myServer;
	(void)pmsg;
	return Sdf_co_success;
}

Sdf_ty_retVal DTMF_MessageStack::sendMessage(void* pmsg,int msglen,DatagramSocket *m_myServer)
{
	(void)m_myServer;
	(void)pmsg;
	return Sdf_co_success;
}


//Added  by Qunajie.Deng s  2105.6.5
Sdf_ty_retVal Dtmf_ProxyMessageStack::handleMessage(void * pmsg, DatagramSocket * m_myServer)
{
	//step-1:change the req type.
	if(NULL == pmsg)
	{
		clm_log(CLM_ERROR,"Dtmf_ProxyMessageStack-handleMessage: pmsg is null!");
		return  Sdf_co_fail;
	}
    app_dtmf_t * app_msg = (app_dtmf_t*)pmsg;
    int dtmf_signal;
    int re_val=-1;
    long src_port =app_msg->rcv_port;
    long timestamp = 0;
    re_val = PxyReqRecordInfo::getInstance()->handle_msgUDP((char*)app_msg->rcv_msg, &dtmf_signal,&timestamp);
    if(0== re_val)
    {
        ;
    }
    else if(re_val == 1)
    {
        char to_msg[1024] = "";
        sprintf(to_msg,"%d:%ld:%ld",dtmf_signal,src_port,timestamp);
#ifdef  SHOW_DTMF_INFO
        printf("to_msg:[%s]\n",to_msg);
#endif
        clm_log(CLM_INFO,"to_rtp_msg:[%s]",to_msg);
	    long  rel_size = 0;
        rel_size = m_myServer->send(to_msg,strlen(to_msg),PxyReqRecordInfo::getInstance()->RTPProxyoutaddr);
	 if(rel_size<=0)
	 {
	 	clm_log(CLM_ERROR,"send_to  rtp is faile!");
	 }
	 else
	 {
	 ;
	 }
    }
    delete app_msg;
    
    
}


//Added  by Quanjie.Deng e  2015.6.5



