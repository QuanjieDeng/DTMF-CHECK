#include "sdfmessagequeue.h"
#include "sdfthreadpool.h"
#include "Dtmf.h"
#include "XML.h"
#include "Dtmf_MessageStack.h"
#include <stdio.h>
#include <unistd.h>
#include "rtpptoxy_reqInfo.h"
//---------------------------------------------
//FUNC:
//DESCRIPTION:
//---------------------------------------------
DTMFMessageManager* DTMFMessageManager::instance = NULL;

Sdf_ty_retVal DTMFMessageManager::init()
{
	printf("start init the  DTMFServer!\n");
	try
    {
		char * pTemp = NULL;
		memset(RTPPROXY_ip, 0x00, 32);
        memset(localip, 0x00, 32);
        
        string log_serverip ;
		CXML::getInstance()->getFirstElementValue("remotelogserverip", log_serverip);//
	 
		string a;

		CXML::getInstance()->getNextElementValue("local_ip", a);
		strcpy(localip,a.c_str());
			
		CXML::getInstance()->getNextElementValue("local_port", a);
		localPort= atoi(a.c_str());
		
		CXML::getInstance()->getNextElementValue("RTPPROXY_ip", a);
		strcpy(RTPPROXY_ip,a.c_str());

		CXML::getInstance()->getNextElementValue("RTPPROXY_port", a);
		RTPPROXY_port= atoi(a.c_str());	
		printf("init:local:[%s][%d]\n",localip,localPort);
		printf("init:rtpproxy:[%s][%ld]\n",RTPPROXY_ip,RTPPROXY_port);

        //³õÊ¼»¯RTPµØÖ·
        PxyReqRecordInfo::getInstance()->RTPProxyoutaddr = (sockaddr_in*)malloc( sizeof(sockaddr_in));
        memset(PxyReqRecordInfo::getInstance()->RTPProxyoutaddr, 0, sizeof(sockaddr_in));

        PxyReqRecordInfo::getInstance()->RTPProxyoutaddr->sin_family = AF_INET;
        PxyReqRecordInfo::getInstance()->RTPProxyoutaddr->sin_addr.s_addr = inet_addr(RTPPROXY_ip);
        PxyReqRecordInfo::getInstance()->RTPProxyoutaddr->sin_port = htons(RTPPROXY_port);  
		//m_myServer = new DatagramSocket(localPort, localip,RTPPROXY_port,RTPPROXY_ip, FALSE, TRUE);
        m_myServer = new DatagramSocket(localPort, localip, FALSE, TRUE);
		m_myServer->UDPRecvStartThread();
		
		this->start();
		printf("DTMFServer init succed!\n");
	}
	catch(...)
	{
	        return Sdf_co_fail;
	}
	return Sdf_co_success;
}

//---------------------------------------------
//FUNC:performTask
//DESCRIPTION:  the work function, to handle the msg 
//in our messageQuenue.
//---------------------------------------------
void DTMFMessageManager::performTask(void * pData)
{
	if ( NULL ==  pData )
	{
	    return;
	}

	dtmf_msgFormat* msg    = (dtmf_msgFormat*)pData;
	do
	{
		 DTMF_MessageStack * pObj = getHandleObj(msg->msgType);		
		if(Sdf_co_success != pObj->handleMessage(msg->pMsgData,m_myServer));
		{
			break;
		}
		
	}while(0);
	if ( NULL != msg )
	{
	    delete msg;
	    msg = NULL;
	}
}

 DTMF_MessageStack* DTMFMessageManager::getHandleObj(Sdf_ty_U32bit type)
{
	switch ( type )
	{
		case DTMF_FromProxyMsg:
		{
			return Dtmf_ProxyMessageStack::getInstance();
		}
		default:
		{
			return NULL;
		} 
		return NULL;
	}
	
}


