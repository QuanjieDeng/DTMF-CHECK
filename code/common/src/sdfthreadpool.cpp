/*******************************************************************************
 * FILE NAME:   sdfthreadpool.cpp 
 *
 * DESCRIPTION: This file has ThreadPool class declaration. This class can be
 * derived & customizable to perform required tasks without bothering to manage
 * pool of threads.
 ******************************************************************************/
#include "sdfthreadpool.h"
#include <sys/syscall.h>
#include <unistd.h>

#include <stdio.h>
#include <sys/time.h>

// Define the memory allocation override methods for the
// Sdf_cl_commonThreadPool class

/*******************************************************************************
** FUNCTION:    threadPoolWorkerThreadLauncher
**
** DESCRIPTION: The Thread Pool Worker Thread launcher Function.
*******************************************************************************/
void* threadPoolWorkerThreadLauncher\
	(\
	void *pData\
	)
{

	Sdf_cl_commonThreadPool *pLaunchSt = (Sdf_cl_commonThreadPool*)pData;
	printf("thread num <%d> number is <%ld>\n",pLaunchSt->numberOfWorkers,syscall(__NR_gettid));
	/* Hand over control to the worker method of the callhandler. */
	pLaunchSt->startWorker();
	return NULL;
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadPool :: Sdf_cl_commonThreadPool
**
** DESCRIPTION: Constructor instantiates the thread pool.
*******************************************************************************/
Sdf_cl_commonThreadPool :: Sdf_cl_commonThreadPool\
	(Sdf_cl_commonMessageQueue 	*pQueue,\
	Sdf_ty_S32bit 			numWorker
	)
{
	this->pMessageQueue = pQueue;
	/* 
	 * Following is useful in registrar component case, where in for functional
	 * interface based construction, the queue passed to base class is null.
	 */
	if ( NULL == this->pMessageQueue )
	{
		this->numberOfWorkers = 0;
		return;
	}
	this->numberOfWorkers = numWorker;
	/* Initialize list that maintains status of the worker threads. */
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadPool :: ~Sdf_cl_commonThreadPool
**
** DESCRIPTION: Destructor.
*******************************************************************************/
Sdf_cl_commonThreadPool :: ~Sdf_cl_commonThreadPool(void)
{
	/* Any other data destruction. */
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadPool :: start
**
** DESCRIPTION: This spawns the number of Workers threads given by the 
**				numberOfWorkers member variable of the thread pool class object.
*******************************************************************************/
void Sdf_cl_commonThreadPool :: start()
{
	printf("entry Sdf_cl_commonThreadPool :: start\n");
	/* 
	 * It is possible that this interface is called by mistake, in which
	 * case it returns without creating the thread.
	 */
	if ( NULL == this->pMessageQueue )
	{
		printf("Sdf_co_null == this->pMessageQueue\n");
		return;
	}
	
	Sdf_ty_S32bit numThreadsToBeCreated = 0;
	numThreadsToBeCreated =  this->numberOfWorkers - this->threadIdList.size();
	printf("numThreadsToBeCreated = %d\n",numThreadsToBeCreated);
	for ( Sdf_ty_S32bit i = 0; i < numThreadsToBeCreated; i++ )
	{ 
		pthread_t* pThreadId = new pthread_t;
		
		if ( 0 != pthread_create(pThreadId, NULL, \
			&threadPoolWorkerThreadLauncher, this) )
		{
			printf("Sdf_fn_createThread Sdf_co_fail\n");
			i--;
			delete pThreadId;
			continue;
		}
		this->threadIdList.push_front(pThreadId);
	}
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadPool :: stop
**
** DESCRIPTION: This method deletes all the worker threads of the thread pool. 
**				To resume operation after stopping a threadpool, worker threads
**				must be added using addWorker, or using	the start method.
*******************************************************************************/
void Sdf_cl_commonThreadPool :: stop()
{
	printf("entry Sdf_cl_commonThreadPool :: stop\n");
	/* 
	 * Since no threads are started in following case & no need of waiting to
	 * kill non-existent threads.
	 */
	if ( NULL == this->pMessageQueue )
		return;

	/* Call deleteWorker interface to delete all the worker threads. */
	this->deleteWorker(this->numberOfWorkers);
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadPool :: startWorker
**
** DESCRIPTION: INTERNAL: Function that does whatever a worker thread is 
**				supposed to do. Calls performTask on each message.
*******************************************************************************/
void Sdf_cl_commonThreadPool :: startWorker()
{
	printf("entry Sdf_cl_commonThreadPool :: startWorker\n");
	void* pData = NULL;
	for(;;)
	{
		/* Read from the Queue. Process the data once it is available. */
		pData = this->pMessageQueue->read();//Sdf_cl_cmnComponentRegistration::post will send msg and read will return
		if ( NULL != pData )
		{
			this->performTask(pData);
		}
		else
		{
			continue;
		}
		pData = NULL;
	}
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadPool :: addWorker
**
** DESCRIPTION: Spawns new worker threads. The method returns the total number
**				of worker threads in the pool after the addition.
*******************************************************************************/
Sdf_ty_S32bit Sdf_cl_commonThreadPool :: addWorker
	(
	Sdf_ty_S32bit addNum
	)
{
	/* 
	 * Since no threads are started in following case & no need of starting 
	 * new threads.
	 */
	if ( NULL == this->pMessageQueue )
		return 0;
		
	/* Initialize list that maintains status of the worker threads. */
	for ( Sdf_ty_S32bit i = 0; i < addNum; i++ )
	{
		pthread_t* pTid = new pthread_t;

		if ( 0 != pthread_create(pTid, NULL, \
			&threadPoolWorkerThreadLauncher, this) )
		{
			delete pTid;
			i--;
			continue;
		}
		
		this->threadIdList.push_front(pTid);
		this->numberOfWorkers++;
	}
	return this->numberOfWorkers;
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadPool :: deleteWorker
**
** DESCRIPTION: This method deletes worker threads form the thread-pool. The 
**				threads deleted are idle threads. The method blocks until it 
**				can find enough idle threads to delete.
*******************************************************************************/
Sdf_ty_S32bit Sdf_cl_commonThreadPool :: deleteWorker\
	(\
	Sdf_ty_S32bit delNum\
	)
{
	/* Since no threads are started in following case & no need of deleting. */
	if ( NULL == this->pMessageQueue )
		return 0;
	
	Sdf_ty_S32bit tobeDeleted = delNum;
	void* pExitStatus;
	if ( this->numberOfWorkers < delNum )
	{
		delNum      = this->numberOfWorkers;
		tobeDeleted = delNum;
	}
		
	/* Initialize list that maintains status of the worker threads. */
	while ( 0 != this->threadIdList.size() )
	{
		pthread_t* pThreadId = (pthread_t*)\
			this->threadIdList.back();
		this->threadIdList.pop_back();

		if ( NULL != pThreadId )
		{
			pthread_cancel(*pThreadId);
			pthread_join(*pThreadId, &pExitStatus);
			delete pThreadId;
			if ( 0 == --delNum )
				break;
		}
	}
	this->numberOfWorkers = this->numberOfWorkers - tobeDeleted;
	return this->numberOfWorkers;
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadPool :: getNumOfWorker
**
** DESCRIPTION: Get number threads in the thread-pool.
*******************************************************************************/
Sdf_ty_S32bit Sdf_cl_commonThreadPool :: getNumOfWorker()
{
	return this->numberOfWorkers;
}

Sdf_ty_S32bit Sdf_cl_commonThreadPool::getQueueSize()
{
	if(pMessageQueue!=NULL)
		return pMessageQueue->size();
	return 0;
}


