#ifndef __RD_CDR_H__
#define __RD_CDR_H__
#include  "sdfmessagequeue.h"
#include  "sdfthreadpool.h"
#include  "DatagramSocket.h"
#include  "Dtmf_MessageStack.h"
#include  "common.h"
#include  "clm_log.h"

#include  <string.h>

class DTMFMessageManager : public Sdf_cl_commonThreadPool
{
public:
	#if  1
	int 	localPort ;
	char 	localip[32];
	char 	RTPPROXY_ip[32];
	int 	RTPPROXY_port;
	//sockaddr_in *RTPProxyoutaddr;
	#endif
	static DTMFMessageManager* getInstance()
	{
		if ( instance == NULL )
		{
			instance = new DTMFMessageManager();
		}
		return instance;
	}
	DTMFMessageManager():Sdf_cl_commonThreadPool(NULL, 0)
	{}

    void performTask(void* pData);
	Sdf_ty_retVal init();

	DTMFMessageManager(Sdf_cl_commonMessageQueue *pQueue)\
		:Sdf_cl_commonThreadPool(pQueue, 10)
	{
		m_myServer = NULL;
	};
private:
	DTMF_MessageStack   * getHandleObj(Sdf_ty_U32bit type);
	static DTMFMessageManager *instance;
	DatagramSocket *m_myServer;
};

#endif
