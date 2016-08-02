/******************************************************************************

******************************************************************************/
#ifndef __SDFEXPIRETIMER_H__
#define __SDFEXPIRETIMER_H__

#include "common.h"
#include "../../dtmf/h/clm_log.h"
#include <string>
#include <map>
#include <pthread.h>
using namespace std;

typedef struct 
{
	int   expireTime;
	void* pData;
}S_TimeElement;

typedef struct InfoSession
{
	int   InfoSessionID;
	InfoSession()
	{
		InfoSessionID = 0;
	}
}S_InfoSessionID;




typedef void (*timeoutFuncPtr)(void *);
typedef void (*removeTimeElementPtr)(void*);
typedef map<string, S_TimeElement*>::iterator Iter_Timer;

class Sdf_Timer
{
public:
	Sdf_Timer(int bucketNumber, int interval, 
			timeoutFuncPtr timeoutFun, removeTimeElementPtr removeFun);
	~Sdf_Timer();
    Sdf_ty_retVal run();
	Sdf_ty_retVal startTime(S_TimeElement *pTimer,const char* key);
    Sdf_ty_retVal updateTime(int exptime,const char* key);
    Sdf_ty_retVal stopTime(const char* key);
	
	pthread_mutex_t m_ExpTimerMutex;
	map<string, S_TimeElement*> m_pExpTimerTabl;
	timeoutFuncPtr        timeoutFun;
	removeTimeElementPtr  removeFun;
	int interval;
};

#endif /* __SDFEXPIRETIMER_H__ */
