/*******************************************************************************
 * FILE NAME:   sdfmessagequeue.cpp
 *
 * DESCRIPTION: This file contains the implementation for the message queue 
 * 				class.
 ******************************************************************************/

#include "sdfmessagequeue.h"
#include <string.h>
#include <stdio.h>
void Sdf_fn_threadCancellationHandler(void *pData)
{
	pthread_mutex_t *pMutex = (pthread_mutex_t *)pData;
	pthread_mutex_trylock(pMutex);
	pthread_mutex_unlock(pMutex);
}

#define Sdf_fn_threadCleanupPop(execute) pthread_cleanup_pop(execute)
#define Sdf_fn_threadCleanupPush(routine,arg) pthread_cleanup_push(routine,arg)


// Define the memory allocation override methods for the
// Sdf_cl_commonMessageQueue class

/*******************************************************************************
 ** FUNCTION:    Sdf_cl_commonMessageQueue :: Sdf_cl_commonMessageQueue
 **
 ** DESCRIPTION: This is the Constructor for the class. It initializes mutex and
 **				the conditional variables.
*******************************************************************************/
Sdf_cl_commonMessageQueue :: Sdf_cl_commonMessageQueue()
{
	pthread_mutex_init(&(this->queueLock), NULL);

	this->queueNotEmpty = new pthread_cond_t;
	memset(this->queueNotEmpty, 0, sizeof(pthread_cond_t));
	if ( pthread_cond_init(this->queueNotEmpty, NULL) != 0 )
	{
		delete this->queueNotEmpty;
	}
}	

/*******************************************************************************
 ** FUNCTION:    Sdf_cl_commonMessageQueue :: ~Sdf_cl_commonMessageQueue
 **
 ** DESCRIPTION: This is the Destructor for the class. It delete all elements in
 **				list and destroys mutex and condition variables.
 *******************************************************************************/
Sdf_cl_commonMessageQueue :: ~Sdf_cl_commonMessageQueue()
{
	if ( 0 == this->queue.size() )
		pthread_cond_signal(this->queueNotEmpty);

	/* Destroy the queueLock Mutex. */
	pthread_mutex_destroy(&(this->queueLock));

	delete this->queueNotEmpty;
	this->queueNotEmpty = NULL; 
}

/*******************************************************************************
 ** FUNCTION:    Sdf_cl_commonMessageQueue :: post
 **
 ** DESCRIPTION: This interface posts the message to the MessageQueue.
 *******************************************************************************/
void Sdf_cl_commonMessageQueue :: post\
		 (\
		  void *pData,\
		  bool msgEmergency\
		 )
{
	pthread_mutex_lock(&(this->queueLock));
	// For emergency call messages, post the message to top
	// of the list instead of end of the list for normal calls.
	if ( msgEmergency )
	{
		this->queue.push_front(pData);
	}
	else
	{
		this->queue.push_back(pData);
	}

	pthread_mutex_unlock(&(this->queueLock));
	pthread_cond_signal(this->queueNotEmpty);
}

/*******************************************************************************
 ** FUNCTION:    Sdf_cl_commonMessageQueue :: read
 **
 ** DESCRIPTION: This interface reads the message from the MessageQueue.
 *******************************************************************************/
void* Sdf_cl_commonMessageQueue :: read\
		(\
		 void\
		)
{
	void *retval = NULL;
	pthread_mutex_lock(&(this->queueLock));
	while ( 0 == this->queue.size() )
	{
		Sdf_fn_threadCleanupPush(Sdf_fn_threadCancellationHandler, (void*)(&(this->queueLock)));
		pthread_cond_wait((this->queueNotEmpty), &(this->queueLock));
		Sdf_fn_threadCleanupPop(0);
	}
		
	retval = this->queue.back();
	if ( NULL != retval )
	{
		this->queue.pop_back();
	}
	pthread_mutex_unlock(&(this->queueLock));

	return retval;
}

/*******************************************************************************
 ** FUNCTION:    Sdf_cl_commonMessageQueue :: size
 **
 ** DESCRIPTION: This interface returns number of messages in the MessageQueue.
 *******************************************************************************/
Sdf_ty_S32bit Sdf_cl_commonMessageQueue :: size\
				  (\
				   void\
				  )
{
	return this->queue.size();
}




