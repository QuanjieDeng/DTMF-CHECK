/*******************************************************************************
 * FILE NAME:   sdfmessagequeue.h
 *
 * DESCRIPTION: This file contains the implementation for the message queue 
 * 				class.
 ******************************************************************************/

#ifndef __SDF_MESSAGE_QUEUE_H_
#define __SDF_MESSAGE_QUEUE_H_

#include <pthread.h>
#include "common.h"
#include <list>
typedef std::list<void*> MsgQueue; 

// -----------------------------------------------------------------------------
// This Class implements the Message Queues.
// -----------------------------------------------------------------------------
class Sdf_cl_commonMessageQueue
{

public:
	
	// -------------------------------------------------------------------------
	//	Constructor: Initialises mutex and the conditional variables.
	//
	// 	Parameters are: 
	//	None.
	// -------------------------------------------------------------------------
	Sdf_cl_commonMessageQueue();

	// -------------------------------------------------------------------------
	//	Destructor: It delete all elements in list and destroys mutex and 
	//				condition variables.
	//
	// 	Parameters are: 
	//	None.
	// -------------------------------------------------------------------------
	~Sdf_cl_commonMessageQueue();

	// -------------------------------------------------------------------------
	//	This function posts a message into the queue. Thread synchronization is
	//	done inside the post function. User need not lock queue for MT safe
	//	operation.
	//
	// 	Parameters are: 
	//	pData:
	//		Pointer to the data to be posted to the queue.
	//	msgPriority:
	//		Specifies the priority of the message being posted.
	// -------------------------------------------------------------------------
	void post\
		(\
		void *pData,\
		bool bIsMsgEmergency = false\
		);

	// -------------------------------------------------------------------------
	//	This function reads a message from the queue. Call blocks until a
	//	message is available. Operation is thread safe - user need not lock the
	//	queue for access.
	//
	// 	Parameters are: 
	//	None:
	// -------------------------------------------------------------------------
	void* read\
		(\
		void\
		);
	
	// -------------------------------------------------------------------------
	//	This function retrieves the size of the queue.
	//
	// 	Parameters are: 
	//	None:
	// -------------------------------------------------------------------------
	Sdf_ty_S32bit size\
		(\
		void\
		);

private:
	// -------------------------------------------------------------------------
	//	DESCRIPTION: 	Member Variables.
	//
	//	queue			-	Singly Linked List which will be the queue.
	//	queueLock		-	Mutex to lock the queue.
	//	queueNotEmpty	-	Conditional variable to be signalled to indicate
	//						that the queue is not empty.
	// -------------------------------------------------------------------------
	MsgQueue                        queue;
	pthread_mutex_t                 queueLock;
	pthread_cond_t                  *queueNotEmpty;
};

#endif
