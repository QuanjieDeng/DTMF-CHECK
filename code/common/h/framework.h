#ifndef _CDR_FRAMEWORK_H
#define _CDR_FRAMEWORK_H

#include "sdfmessagequeue.h"
#include "common.h"
#include <map>


typedef enum
{
	COMPONENT_UNKNOW = 0,
	COMPONENT_DTMF    = 1,
	COMPONENT_POLICY = 2,
	COMPONENT_MAX    = 100
}COMPONENT_ID;



class framework
{
public:
	static framework& getInstance();
	/*static framework* getInstance();
	{
		if ( instance == NULL )
		{
			instance = new framework();
		}
		return instance;
	}*/

	bool registerComponent(COMPONENT_ID id, Sdf_cl_commonMessageQueue *pQueue);
	bool post(COMPONENT_ID id, dtmf_msgFormat* pMsg);

private:
	framework();
	void mapLock();
	void mapUnlock();
	
	std::map<int , Sdf_cl_commonMessageQueue*> componentMap;	
	static framework *instance;
	pthread_mutex_t   m_lock;
};

#endif

