/*******************************************************************************
 * FILE NAME:   sdfthreadpool.h
 *
 * DESCRIPTION: This file has ThreadPool class declaration. This class can be
 * derived & customizable to perform required tasks without bothering to manage
 * pool of threads.
 ******************************************************************************/
#ifndef __SDF_THREAD_POOL_H_
#define __SDF_THREAD_POOL_H_

#include "sdfmessagequeue.h"
#include <list>
#include "common.h"
// -----------------------------------------------------------------------------
// This Class implements the Thread Pool.
// -----------------------------------------------------------------------------
class Sdf_cl_commonThreadPool
{
public:
	// -------------------------------------------------------------------------
	//	DESCRIPTION: 	Member Variables.
	//
	//	threadIdList	-	List containing status of worker threads.
	//	numberOfWorkers	-	Number of worker threads in the pool.
	// -------------------------------------------------------------------------
	std::list<pthread_t*> threadIdList;
	Sdf_ty_S32bit numberOfWorkers;
	
	// -------------------------------------------------------------------------
	//	Constructor: Instantiates the thread pool.
	//
	// 	Parameters are: 
	//	pQueue:
	//		Message queue on which tasks to be assigned to the threads are
	//		posted. 
	//
	//	numWorker:
	//		The number of worker threads(the threads that consume the tasks in
	//		the queue) launched is specified by numWorker. This number can be
	//		altered at runtime.
	// -------------------------------------------------------------------------
	Sdf_cl_commonThreadPool\
		(\
		Sdf_cl_commonMessageQueue 	*pQueue,\
		Sdf_ty_S32bit                    numWorker
		);
	
	// -------------------------------------------------------------------------
	//	Destructor: 
	//
	// 	Parameters are: 
	//	None.
	// -------------------------------------------------------------------------
	virtual ~Sdf_cl_commonThreadPool();
	
	// -------------------------------------------------------------------------
	//	This spawns the number of Workers threads given by the numberOfWorkers 
	//	member variable of the thread pool class object.
	//
	// 	Parameters are: 
	//	None.
	// -------------------------------------------------------------------------
	void start();
	
	// -------------------------------------------------------------------------
	//	This method deletes all the worker threads of the thread pool. To
	//	resume operation after stopping a threadpool, worker threads must be
	//	added using addWorker, or using	the start method.
	//
	// 	Parameters are: 
	//	None.
	// -------------------------------------------------------------------------
	void stop();

	// -------------------------------------------------------------------------
	//	INTERNAL: Function that does whatever a worker thread is supposed to do.
	//
	// 	Parameters are: 
	//	None.
	// -------------------------------------------------------------------------
	void startWorker();
	
	// -------------------------------------------------------------------------
	//	Spawns new worker threads. The method returns the total number of
	//	worker threads in the pool after the addition.
	//
	// 	Parameters are: 
	//	numWorker:
	//		Number of worker threads to be added to the pool.
	// -------------------------------------------------------------------------
	Sdf_ty_S32bit addWorker
		(
		Sdf_ty_S32bit numWorker
		);
	
	// -------------------------------------------------------------------------
	//	This method deletes worker threads form the thread-pool. The threads
	//	deleted are idle threads. The method blocks until it can find enough
	//	idle threads to delete.
	//
	// 	Parameters are: 
	//	numWorker:
	//		Number of worker threads to be deleted from the pool.
	// -------------------------------------------------------------------------
	Sdf_ty_S32bit deleteWorker
		(\
		Sdf_ty_S32bit numWorker\
		);
	
	// -------------------------------------------------------------------------
	//	Get number threads in the thread-pool. 
	//	
	// 	Parameters are: 
	// 	None.
	// -------------------------------------------------------------------------
	Sdf_ty_S32bit getNumOfWorker\
		(\
		void\
		);
	
	// -------------------------------------------------------------------------
	//	Classes deriving from the threadpool base class must implement the
	//	performTask interface. This is the function that is called in the worker
	//	threads when a task is pending. The message retrieved from the task
	//	queue is available in this method as pData. Cancellation is disabled
	//	when this method is invoked. The worker thread can only be cancelled
	//	when performTask returns. 
	//	
	// 	Parameters are: 
	// 	pData:
	// 		Message retrieved from the queue.
	// -------------------------------------------------------------------------
	virtual void performTask\
		(\
		void *pData\
		) = 0;

private:
	// -------------------------------------------------------------------------
	//	DESCRIPTION: 	Member Variables.
	//
	//	pMessageQueue	-	The message queue from wich messages are read and
	//						assigned to worker threads.
	// -------------------------------------------------------------------------
	Sdf_cl_commonMessageQueue *pMessageQueue;

public:
	Sdf_ty_S32bit getQueueSize();
};
#endif
