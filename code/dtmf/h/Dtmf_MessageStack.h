#ifndef __RD_POLICYMESSAGESTACK_H__
#define __RD_POLICYMESSAGESTACK_H__

#include "DatagramSocket.h"
#include "sdfhashtable.h"
#include "Dtmf_types.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <pwd.h>
#include <shadow.h>
#include <glob.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/epoll.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <linux/sockios.h>

#define Sdf_CallCdrRecordHashBuckets  10000


class DTMF_MessageStack
{
public:
	DTMF_MessageStack(){};
	
	virtual Sdf_ty_retVal handleMessage(void * pmsg,DatagramSocket *m_myServer);
	virtual Sdf_ty_retVal sendMessage(void* pmsg,int msglen,DatagramSocket *m_myServer);
	Sdf_ty_retVal  	 create_pthread(int  port,void *(*pfunc) (void *));

	virtual ~DTMF_MessageStack(){};
private:
	
};

class Dtmf_ProxyMessageStack:public DTMF_MessageStack
{
public:
	static   Dtmf_ProxyMessageStack* getInstance()
	{
		if ( instance == NULL )
		{
			instance = new Dtmf_ProxyMessageStack();
		}
		return instance;
	}
	Sdf_ty_retVal handleMessage(void * pmsg,DatagramSocket *m_myServer);
	Sdf_ty_retVal		handle_check_add(long port);
	Sdf_ty_retVal		handle_check_drop(long port);
	
private:
	
	static  Dtmf_ProxyMessageStack *instance;
};
void* 			thr_listen_port(void *args);
#endif
