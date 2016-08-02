/******************************************************************************

******************************************************************************/
#include "sdfexpiretimer.h"

/*---------------------------------------------------------------*/
void* sdf_timer_thread(void* pData)
{
	Sdf_ty_error	err;
	Sdf_Timer *pclass= (Sdf_Timer *)pData;

	while(1)
	{
		pthread_mutex_lock(&(pclass->m_ExpTimerMutex));
		Iter_Timer it = pclass->m_pExpTimerTabl.begin();
		//clm_log(CLM_INFO,"expireTime:%d", it->second->expireTime);
	
		for ( ;pclass->m_pExpTimerTabl.end() != it; )
		{
			if ( 0 >= it->second->expireTime )
			{
				clm_log(CLM_INFO,"<timeout>");
				it->second->expireTime -= pclass->interval;
				
				pclass->timeoutFun(it->second->pData);
				pclass->removeFun(it->second);
			    pclass->m_pExpTimerTabl.erase(it++);
			}
			else
			{
				it->second->expireTime -= pclass->interval;
			
				//clm_log(CLM_INFO,"func sdf_timer_thread: expireTime:%d", it->second->expireTime);
				it++;
			}
			
		}
		
		pthread_mutex_unlock(&(pclass->m_ExpTimerMutex));
		//Sdf_fn_sleep(pclass->interval,&err);
		//sleep(1);
	}
	/* end of outer while loop*/
	return Sdf_co_null;
}

Sdf_Timer::Sdf_Timer(int bucketNumber, int interval,
		timeoutFuncPtr timeoutFun, removeTimeElementPtr removeFun)
{
	(void)bucketNumber;
	pthread_mutex_init(&m_ExpTimerMutex, NULL);
	this->interval   = interval;
	this->timeoutFun = timeoutFun;
	this->removeFun  = removeFun;
}

Sdf_Timer::~Sdf_Timer()
{
	pthread_mutex_destroy(&m_ExpTimerMutex);
}

Sdf_ty_retVal Sdf_Timer::run()
{
	Sdf_ty_threadT timerThreadId;
	Sdf_ty_error   thError;

	/* Creating the expire timer thread  */
	//Sdf_fn_createThread(&(timerThreadId),Sdf_co_null,sdf_timer_thread,(void *)this,&thError);
	//Sdf_fn_createThread(&(timerThreadId),Sdf_co_null,sdf_timer_thread,(void *)this,&thError);//the old  style is  up
        pthread_create(&(timerThreadId),Sdf_co_null,sdf_timer_thread,(void *)this);
	return Sdf_co_success;

}
Sdf_ty_retVal Sdf_Timer::startTime(S_TimeElement *pTimer,const char* key)
{
	if  ( NULL == pTimer || NULL == key )
	{
	    return Sdf_co_fail;
	}
	pthread_mutex_lock(&m_ExpTimerMutex);
	Iter_Timer it = m_pExpTimerTabl.find(key);

	if ( m_pExpTimerTabl.end() != it )
	{
		pthread_mutex_unlock(&m_ExpTimerMutex);
		return Sdf_co_fail;
	}
	
	m_pExpTimerTabl[key] = pTimer;
	pthread_mutex_unlock(&m_ExpTimerMutex);
	clm_log(CLM_INFO,"keepalive_startTimeer successful %d ",pTimer->expireTime);
	return Sdf_co_success;
}



Sdf_ty_retVal Sdf_Timer::updateTime(int exptime, const char* key)
{
	clm_log(CLM_INFO, "enty func updatetime--keepalive");	
	if ( NULL == key )
	{
		clm_log(CLM_INFO, "The key of updatetime is NULL");
	    return Sdf_co_fail;
	}
	
	pthread_mutex_lock(&m_ExpTimerMutex);
	Iter_Timer it = m_pExpTimerTabl.find(key);
	
	if ( m_pExpTimerTabl.end() == it )
	{
		clm_log(CLM_INFO, "func-updateTime::m_pExpTimerTabl.end() == it");
		pthread_mutex_unlock(&m_ExpTimerMutex);
		return Sdf_co_fail;
	}
	
	it->second->expireTime = exptime;
	pthread_mutex_unlock(&m_ExpTimerMutex);
	clm_log(CLM_INFO,"keepalive_updateTime successful %d ",exptime);
	clm_log(CLM_INFO,"keepalive_updateTime successful");
	return Sdf_co_success;
}




Sdf_ty_retVal Sdf_Timer::stopTime(const char* key)
{
	if ( NULL == key )
	{
	    return Sdf_co_fail;
	}
	pthread_mutex_lock(&m_ExpTimerMutex);
	Iter_Timer it = m_pExpTimerTabl.find(key);

	if ( m_pExpTimerTabl.end() == it )
	{
		pthread_mutex_unlock(&m_ExpTimerMutex);
		return Sdf_co_fail;
	}
	this->removeFun(it->second);
	m_pExpTimerTabl.erase(it);
	pthread_mutex_unlock(&m_ExpTimerMutex);
	return Sdf_co_success;

}
