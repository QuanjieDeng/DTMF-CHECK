#include "framework.h"
#include  <stdio.h>

framework* framework::instance = NULL;

framework& framework::getInstance()
{
	if (Sdf_co_null == instance)
		instance = new framework();

	return *instance;	
}

framework::framework()
{
	pthread_mutex_init(&m_lock, NULL);
}

void framework::mapLock()
{
	pthread_mutex_lock(&m_lock);
}

void framework::mapUnlock()
{
	pthread_mutex_unlock(&m_lock);
}

bool framework::registerComponent(COMPONENT_ID id, Sdf_cl_commonMessageQueue *pQueue)
{
	mapLock();
	bool result = false;
	std::map<int , Sdf_cl_commonMessageQueue*> :: iterator it = componentMap.find(id);
	if ( it == componentMap.end() )		
	{
		componentMap[id] = pQueue;
		result = true;
	}
	mapUnlock();
	return result;
}

bool framework::post(COMPONENT_ID id, dtmf_msgFormat* pMsg)
{
	mapLock();
       // printf("post msgType:%d\n",pMsg->msgType);
        bool result = false;
        std::map<int , Sdf_cl_commonMessageQueue*> :: iterator it = componentMap.find(id);
        if ( it != componentMap.end() )
        {
                //printf("post msgType22222222222:%d\n",pMsg->msgType);
                it->second->post(pMsg);
                result = true;
        }
        mapUnlock();
        return result;

}

