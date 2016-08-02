#include "sdfhashtable.h"
#include "string.h"
#include  "../../dtmf/h/clm_log.h"

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonHash :: Sdf_cl_commonHash
**
** DESCRIPTION: This is the Constructor for the class. It initializes all member
** 				variables and assigns all its input parameters to the member 
** 				variables.
*******************************************************************************/
Sdf_cl_commonHash :: Sdf_cl_commonHash\
	(\
	Sdf_ty_hashFunc 			fpHashFunction,\
	Sdf_ty_hashKeyCompareFunc 	fpCompareFunction, \
	Sdf_ty_hashKeyFreeFunc 		fpKeyFreeFunc, \
	Sdf_ty_hashElemFreeFunc 	fpElemFreeFunc, \
	Sdf_ty_U32bit 				numBuckets
	)
{
	/* Initialize member variables. */
	this->fpHashFunc 			= fpHashFunction;
	this->fpCompareFunc 		= fpCompareFunction;
	this->fpHashKeyFreeFunc 	= fpKeyFreeFunc;
	this->fpHashElemFreeFunc 	= fpElemFreeFunc;
	this->numberOfBuckets 		= numBuckets;
	this->numberOfElements 		= 0;
	
	/* Allocate space for buckets. */
	this->ppHashChains = new Sdf_st_commonHashElement*[numBuckets];
	this->bucketCounter =  new Sdf_ty_U32bit[numBuckets];

	/* Initialize the buckets. */
	for (Sdf_ty_U32bit i = 0; i < numBuckets; i++)
	{
		this->ppHashChains[i] = Sdf_co_null;
		this->bucketCounter[i] = 1;
	}
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonHash:: ~Sdf_cl_commonHash
**
** DESCRIPTION: This is the Destructor for the class. It deletes all the 
**				elements in the hash table.
*******************************************************************************/
Sdf_cl_commonHash :: ~Sdf_cl_commonHash\
	(\
	void\
	)
{
	/* 
	 * Iterate through the entire hash table and remove all the elements. Use
	 * the fpHashElemFreeFunc to free all the elements removed. Finally delete
	 * all the buckets.
	 */
	Sdf_st_commonHashIterator iterator;
	this->initIterator(&iterator);
	while (Sdf_co_null != iterator.pCurrentElement)
	{
		void* pKey 	= iterator.pCurrentElement->pKey;
		Sdf_ty_U32bit dCorrId = iterator.pCurrentElement->correlationID;
		this->next(&iterator);
		void *pData = Sdf_co_null;
		if(dCorrId == 0)
			pData = this->remove(pKey);
		else
			pData = this->remove(dCorrId);
		
		if ((Sdf_co_null != pData) && (Sdf_co_null != this->fpHashElemFreeFunc))
			this->fpHashElemFreeFunc(pData);
	}
	delete[] (this->ppHashChains);
	delete[] this->bucketCounter ;
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonHash:: add
**
** DESCRIPTION: This method is used to add elements to the hash table.
*******************************************************************************/
void Sdf_cl_commonHash :: add\
	(\
	void 	*pElement,\
	void	*pKey\
	)
{
	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = this->fpHashFunc(pKey);
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
	/* Allocate and initialize element holder. */
	Sdf_st_commonHashElement* pNewElement = new Sdf_st_commonHashElement;
	pNewElement->pElement = pElement;
	/*increment the total number of elements in the hash table. */
	(this->numberOfElements)++;
	pNewElement->pKey = pKey;
	/* Push element into the bucket. */
	pNewElement->pNext = this->ppHashChains[bucket];
	pNewElement->dRefCount = 1;
	this->ppHashChains[bucket] = pNewElement;
}


/*******************************************************************************
** FUNCTION:    Sdf_cl_commonHash:: add
**
** DESCRIPTION: This method is used to add elements to the hash table.
*******************************************************************************/
void Sdf_cl_commonHash :: add\
	(\
	void 	*pElement,\
	void	*pKey,\
	Sdf_ty_U32bit	*pCorrelationID\
	)
{
	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = this->fpHashFunc(pKey);
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;

	/* Allocate and initialize element holder. */
	Sdf_st_commonHashElement* pNewElement = new Sdf_st_commonHashElement;
	pNewElement->pElement = pElement;
	/*increment the total number of elements in the hash table. */
	(this->numberOfElements)++;
	pNewElement->pKey = pKey;
	/* Push element into the bucket. */
	pNewElement->pNext = this->ppHashChains[bucket];
	pNewElement->dRefCount = 1;

	Sdf_ty_U32bit corrID=0;

	/*
	 * The correlation ID is generated as per following arithmatic.
	 * correlation ID = (bucketCounter * numberOfBuckets) + bucketIndex
	 *
	 * If correlation ID exceeds Sdf_co_u32Max, then the bucketCounter must
	 * be re-initialized to 1.
	 * 
	 * Therefore, correlation ID <= Sdf_co_u32Max
	 * => (bucketCounter * numberOfBuckets) + bucketIndex <= Sdf_co_u32Max
	 * => (bucketCounter * numberOfBuckets) <= (u32Max - bucketIndex)
	 * => bucketCounter <= ((u32Max - bucketIndex)/numberOfBuckets)
	 * 
	 * Therefore if bucketCounter > ((u32Max - bucketIndex)/numberOfBuckets)
	 * RESET bucketCounter to "1".
	 */
	
	if ((this->bucketCounter[bucket]) > \
		((Sdf_co_u32Max - bucket)/this->numberOfBuckets))
	{
		this->bucketCounter[bucket] = 1;
	}
	corrID = (this->bucketCounter[bucket] * this->numberOfBuckets) + bucket;
	
	/* Increment the corr ID generating counter. */
	(this->bucketCounter[bucket])++;

	/* Add the correlation-ID in the hash element */
	pNewElement->correlationID= corrID;

	/* Return the correlation id */
	*pCorrelationID = corrID;

	this->ppHashChains[bucket] = pNewElement;
}


/*******************************************************************************
** FUNCTION:    Sdf_cl_commonHash:: addS
**
** DESCRIPTION: This method is used to add elements to the hash table. The 
** addition is done to the end of the list where the add interface prepends
** the new hash elements.
*******************************************************************************/
void Sdf_cl_commonHash :: addS\
	(\
	void 	*pElement,\
	void	*pKey\
	)
{
	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = this->fpHashFunc(pKey);
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
	/* Allocate and initialize element holder. */
	Sdf_st_commonHashElement* pNewElement = new Sdf_st_commonHashElement;
	pNewElement->pElement = pElement;
	pNewElement->pNext = Sdf_co_null;
	pNewElement->dRefCount = 1;
	/*increment the total number of elements in the hash table. */
	(this->numberOfElements)++;
	pNewElement->pKey = pKey;

	/*
	 * Check whether the element being added is the first element.
	 * In this case the value of the pNext of the bucket should point
	 * to the element being added.
	 */
	if(this->ppHashChains[bucket] == Sdf_co_null)
	{
		this->ppHashChains[bucket] = pNewElement;
	}
	else
	{
		/* Traverse through the list and add the new element to the end */
		Sdf_st_commonHashElement *pIterator = this->ppHashChains[bucket];
		while(pIterator->pNext != Sdf_co_null)
			pIterator = pIterator->pNext;

		pIterator->pNext = pNewElement;
	}
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonHash:: addS
**
** DESCRIPTION: This method is used to add elements to the hash table. The 
** addition is done to the end of the list where the add interface prepends
** the new hash elements.
*******************************************************************************/
void Sdf_cl_commonHash :: addS\
	(\
	void 	*pElement,\
	void	*pKey,\
	Sdf_ty_U32bit	*pCorrelationID\
	)
{
	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = this->fpHashFunc(pKey);
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
	/* Allocate and initialize element holder. */
	Sdf_st_commonHashElement* pNewElement = new Sdf_st_commonHashElement;
	pNewElement->pElement = pElement;
	pNewElement->pNext = Sdf_co_null;
	pNewElement->dRefCount = 1;
	/*increment the total number of elements in the hash table. */
	(this->numberOfElements)++;
	pNewElement->pKey = pKey;

	Sdf_ty_U32bit corrID=0;

	/*
	 * The correlation ID is generated as per following arithmatic.
	 * correlation ID = (bucketCounter * numberOfBuckets) + bucketIndex
	 *
	 * If correlation ID exceeds Sdf_co_u32Max, then the bucketCounter must
	 * be re-initialized to 1.
	 * 
	 * Therefore, correlation ID <= Sdf_co_u32Max
	 * => (bucketCounter * numberOfBuckets) + bucketIndex <= Sdf_co_u32Max
	 * => (bucketCounter * numberOfBuckets) <= (u32Max - bucketIndex)
	 * => bucketCounter <= ((u32Max - bucketIndex)/numberOfBuckets)
	 * 
	 * Therefore, if bucketCounter > ((u32Max - bucketIndex)/numberOfBuckets)
	 * RESET bucketCounter to "1".
	 */
	
	if ((this->bucketCounter[bucket]) > \
		((Sdf_co_u32Max - bucket)/this->numberOfBuckets))
	{
		this->bucketCounter[bucket] = 1;
	}
	corrID = (this->bucketCounter[bucket] * this->numberOfBuckets) + bucket;
	
	/* Increment the corr ID generating counter. */
	(this->bucketCounter[bucket])++;

	/* Add the correlation-ID in the hash element */
	pNewElement->correlationID= corrID;

	/* Return the correlation id */
	*pCorrelationID = corrID;

	/*
	 * Check whether the element being added is the first element.
	 * In this case the value of the pNext of the bucket should point
	 * to the element being added.
	 */
	if(this->ppHashChains[bucket] == Sdf_co_null)
	{
		this->ppHashChains[bucket] = pNewElement;
	}
	else
	{
		/* Traverse through the list and add the new element to the end */
		Sdf_st_commonHashElement *pIterator = this->ppHashChains[bucket];
		while(pIterator->pNext != Sdf_co_null)
			pIterator = pIterator->pNext;

		pIterator->pNext = pNewElement;
	}
}

//addUniqueunlock

Sdf_ty_retVal Sdf_cl_commonHash::addUniqueunlock(
        void                                *pElement,
        void                                *pKey,
        Sdf_ty_error                        *pEcode )
{
    Sdf_ty_retVal                           retval              = Sdf_co_fail;
    clm_log(CLM_INFO,"Sdf_cl_commonHash::addUniqueunlock");
    // Compute hash for the element
    Sdf_ty_U32bit hashKey = this->fpHashFunc(pKey);
    // Locate the bucket
    Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
    // now check to see if this element has already been inserted in the hash
    Sdf_st_commonHashElement *pNextElem = this->ppHashChains[bucket];
    while( Sdf_co_null != pNextElem )
    {
        if( Sdf_co_success == this->fpCompareFunc(pNextElem->pKey, pKey) )
        {
            break;
        }
        pNextElem = pNextElem->pNext;
    }
    if( Sdf_co_null == pNextElem )
    {
        // Element has not been found. Go ahead
        // Allocate and initialize element holder
        Sdf_st_commonHashElement *pNewElement = new Sdf_st_commonHashElement;
        pNewElement->pElement = pElement;
        //increment the total number of elements in the hash table
        (this->numberOfElements)++;
        pNewElement->pKey = pKey;
        // Push element into the bucket
        pNewElement->pNext = this->ppHashChains[bucket];
        this->ppHashChains[bucket] = pNewElement;
        retval = Sdf_co_success;
    }
    else
    {
        // Element already exists. Return  failure
        *pEcode = Sdf_en_errorDuplicateData;
    }
    
    return retval;
}

/*******************************************************************************
** Function     : Sdf_cl_commonHash::addUnique                                **
**                                                                            **
** Description  : This method is used to add unique elements to the hash table**
**                                                                            **
*******************************************************************************/
Sdf_ty_retVal Sdf_cl_commonHash::addUnique(
        void                                *pElement,
        void                                *pKey,
        Sdf_ty_error                        *pEcode )
{
    Sdf_ty_retVal                           retval              = Sdf_co_fail;

    // Compute hash for the element
    Sdf_ty_U32bit hashKey = this->fpHashFunc(pKey);
    // Locate the bucket
    Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
    // now check to see if this element has already been inserted in the hash
    Sdf_st_commonHashElement *pNextElem = this->ppHashChains[bucket];
    while( Sdf_co_null != pNextElem )
    {
        if( Sdf_co_success == this->fpCompareFunc(pNextElem->pKey, pKey) )
        {
            break;
        }
        pNextElem = pNextElem->pNext;
    }
    if( Sdf_co_null == pNextElem )
    {
        // Element has not been found. Go ahead
        // Allocate and initialize element holder
        Sdf_st_commonHashElement *pNewElement = new Sdf_st_commonHashElement;
        pNewElement->pElement = pElement;
        //increment the total number of elements in the hash table
        (this->numberOfElements)++;
        pNewElement->pKey = pKey;
        // Push element into the bucket
        pNewElement->pNext = this->ppHashChains[bucket];
        this->ppHashChains[bucket] = pNewElement;
        retval = Sdf_co_success;
    }
    else
    {
        // Element already exists. Return  failure
        *pEcode = Sdf_en_errorDuplicateData;
    }
    
    return retval;
}
/*DONE ADD CODE*/
/*******************************************************************************
** Function     : Sdf_cl_commonHash::addUnique                                **
**                                                                            **
** Description  : This method is used to add unique elements to the hash table**
**				  It returns a correlation ID
**                                                                            **
*******************************************************************************/
Sdf_ty_retVal Sdf_cl_commonHash::addUnique(
        void                                *pElement,
        void                                *pKey,
		Sdf_ty_U32bit						*pCorrelationID,
        Sdf_ty_error                        *pEcode )
{


		/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = this->fpHashFunc(pKey);
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
    Sdf_ty_retVal retval= Sdf_co_fail;

	// now check to see if this element has already been inserted in the hash
    // first do a bucket level lock
    Sdf_st_commonHashElement *pNextElem = this->ppHashChains[bucket];
    while( Sdf_co_null != pNextElem )
    {
        if( Sdf_co_success == this->fpCompareFunc(pNextElem->pKey, pKey) )
        {
            break;
        }
        pNextElem = pNextElem->pNext;
    }
    if( Sdf_co_null == pNextElem )
    {
        // Element has not been found. Go ahead
        // Allocate and initialize element holder
        Sdf_st_commonHashElement *pNewElement = new Sdf_st_commonHashElement;
        pNewElement->pElement = pElement;
        pNewElement->pKey = pKey;
        // Initialize reference count of hash element to 1
        pNewElement->dRefCount = 1;
		 // Push element into the bucket
        pNewElement->pNext = this->ppHashChains[bucket];

		Sdf_ty_U32bit corrID=0;

		/*
		 * The correlation ID is generated as per following arithmatic.
		 * correlation ID = (bucketCounter * numberOfBuckets) + bucketIndex
		 *
		 * If correlation ID exceeds Sdf_co_u32Max, then the bucketCounter must
		 * be re-initialized to 1.
		 * 
		 * Therefore, correlation ID <= Sdf_co_u32Max
		 * => (bucketCounter * numberOfBuckets) + bucketIndex <= Sdf_co_u32Max
		 * => (bucketCounter * numberOfBuckets) <= (u32Max - bucketIndex)
		 * => bucketCounter <= ((u32Max - bucketIndex)/numberOfBuckets)
		 * 
		 * Therefore if bucketCounter > ((u32Max - bucketIndex)/numberOfBuckets)
		 * RESET bucketCounter to "1".
		 */
		
		if ((this->bucketCounter[bucket]) > \
			((Sdf_co_u32Max - bucket)/this->numberOfBuckets))
		{
			this->bucketCounter[bucket] = 1;
		}
		corrID = (this->bucketCounter[bucket] * this->numberOfBuckets) + bucket;
		
		/* Increment the corr ID generating counter. */
		(this->bucketCounter[bucket])++;

		/* Add the correlation-ID in the hash element */
		pNewElement->correlationID= corrID;

		/* Return the correlation id */
		*pCorrelationID = corrID;

		 // Push element into the bucket
		this->ppHashChains[bucket] = pNewElement;

        //increment the total number of elements in the hash table
        (this->numberOfElements)++;
        retval = Sdf_co_success;
    }
    else
    {
        // Element already exists. Return  failure
        *pEcode = Sdf_en_errorDuplicateData;
    }
    return retval;
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonHash:: fetch
**
** DESCRIPTION: This method fetches the element from the hash table.
*******************************************************************************/
void* Sdf_cl_commonHash :: fetch(void *pKey)
{
	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = fpHashFunc(pKey);
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
	/* Go through chain. */
	Sdf_st_commonHashElement* pIterator = this->ppHashChains[bucket];
	while (Sdf_co_null != pIterator)
	{
		if (Sdf_co_success == this->fpCompareFunc(pIterator->pKey, pKey))
			break;
		pIterator = pIterator->pNext;
	}
	/* Return the element fetched from the hash. */
	return (pIterator != Sdf_co_null)?	pIterator->pElement : Sdf_co_null;
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonHash:: fetch
**
** DESCRIPTION: This method fetches the element from the hash table.
*******************************************************************************/
void* Sdf_cl_commonHash :: fetch(Sdf_ty_U32bit correlationID)
{
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = correlationID % this->numberOfBuckets;
	/* Go through chain. */
	Sdf_st_commonHashElement* pIterator = this->ppHashChains[bucket];
	while (Sdf_co_null != pIterator)
	{
		if (pIterator->correlationID == correlationID)
			break;
		pIterator = pIterator->pNext;
	}
	/* Return the element fetched from the hash. */
	return (pIterator != Sdf_co_null)?	pIterator->pElement : Sdf_co_null;
}

/******************************************************************************
 ** Function:	Sdf_cl_commonHash :: add
 ** Description: The other flavours of add interface take a key and generate a 
 ** session id using the the bucket counter. This method takes a correlationId 
 ** and uses that to insert the element into the hash table. Further this
 ** interface will reset the bucket counter based on session id that has
 ** been passed to ensure that the same id does not get generated again
 *****************************************************************************/
void Sdf_cl_commonHash :: add\
	(\
	 Sdf_ty_U32bit		dCorrelationId,\
	 void*				pKey,\
	 void*				pData\
	)
{

	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = 0;
	if(0 != dCorrelationId)
		bucket = dCorrelationId % this->numberOfBuckets;
	else
		bucket = 0;
	/* Allocate and initialize element holder. */
	Sdf_st_commonHashElement* pNewElement = new Sdf_st_commonHashElement;
	pNewElement->pElement = pData;
	pNewElement->pKey = pKey;
	/* Initialize reference count of hash element to 1. */
	pNewElement->dRefCount = 1;
	pNewElement->correlationID = dCorrelationId;
	
	/* Push element into the bucket. */
	pNewElement->pNext = this->ppHashChains[bucket];
	this->ppHashChains[bucket] = pNewElement;

	/* The bucket counter value has to be re-computed based on the
	 * correlation that is getting added newly. The bucket counter can be
	 * calculated as follows
	 * tempBucketCounter = (dCorrelationId - bucketIndex) / numberOfBuckets
	 * if(tempBucketCounter >= currentBucketCounter)
	 * {
	 * 		Update the counter value to tempBucketCounter + 1. This will
	 * 		ensure that the correlation id generated by this bucket will
	 * 		not overlap with the ones that have already been added
	 * }
	 * Ensure that the newly assigned bucket counter does not generate a
	 * correlation id greater than or equal to Sdf_co_u32Max. If so reset
	 * the same to 1
	 */

	Sdf_ty_U32bit tempBucketCount = 0;

	tempBucketCount = (dCorrelationId - bucket) / this->numberOfBuckets;

	// If the newly computed counter is not greater than the current bucket
	// counter then the same need not be updated.
	if(tempBucketCount >= this->bucketCounter[bucket])
	{
		this->bucketCounter[bucket] = tempBucketCount + 1;

		if ((this->bucketCounter[bucket]) > \
			((Sdf_co_u32Max - bucket)/this->numberOfBuckets))
		{
			this->bucketCounter[bucket] = 1;
		}
	}
	
	/* increment the total number of elements in the hash table. */
	(this->numberOfElements)++;
}


/*******************************************************************************
** FUNCTION:    Sdf_cl_commonHash:: remove
**
** DESCRIPTION: This method removes the element from the hash table.
*******************************************************************************/
void *Sdf_cl_commonHash :: remove\
	(\
	void *pKey\
	)
{
	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = fpHashFunc(pKey);
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
	/* Go through chain. */
	Sdf_st_commonHashElement **ppIterator = &(this->ppHashChains[bucket]);
	while (Sdf_co_null != *ppIterator)
	{
		/* Check whether the references to the key is the same. If so
		 * remove the callobject directly. Do not invoke the key
		 * comparison logic at all.
		 */
		if (((*ppIterator)->pKey == pKey) || \
			Sdf_co_success == this->fpCompareFunc((*ppIterator)->pKey, pKey))
			break;
		ppIterator = &((*ppIterator)->pNext);
	}
	/* If there is no element to remove from the hash table, return NULL. */
	if (Sdf_co_null == *ppIterator)
		return Sdf_co_null;
	/* Remove the element from the Hash table. */	
	void *pRetVal = (*ppIterator)->pElement;
	Sdf_st_commonHashElement *pTempElement = *ppIterator;
	*ppIterator = (*ppIterator)->pNext;
	if (Sdf_co_null != this->fpHashKeyFreeFunc)
	{
		this->fpHashKeyFreeFunc(pTempElement->pKey);
	}
	/* decrement the total number of elements in the hash table. */
	(this->numberOfElements)--;
	delete pTempElement;
	return pRetVal;
}


/*******************************************************************************
** FUNCTION:    Sdf_cl_commonHash:: remove
**
** DESCRIPTION: This method removes the element from the hash table.
*******************************************************************************/
void *Sdf_cl_commonHash :: remove\
	(\
	Sdf_ty_U32bit corrID\
	)
{
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = corrID% this->numberOfBuckets;
	/* Go through chain. */
	Sdf_st_commonHashElement **ppIterator = &(this->ppHashChains[bucket]);
	while (Sdf_co_null != *ppIterator)
	{
		if ((*ppIterator)->correlationID == corrID)
			break;
		ppIterator = &((*ppIterator)->pNext);
	}
	/* If there is no element to remove from the hash table, return NULL. */
	if (Sdf_co_null == *ppIterator)
		return Sdf_co_null;
	/* Remove the element from the Hash table. */	
	void *pRetVal = (*ppIterator)->pElement;
	Sdf_st_commonHashElement *pTempElement = *ppIterator;
	*ppIterator = (*ppIterator)->pNext;
	if (Sdf_co_null != this->fpHashKeyFreeFunc)
	{
		this->fpHashKeyFreeFunc(pTempElement->pKey);
	}
	/* decrement the total number of elements in the hash table. */
	(this->numberOfElements)--;
	delete pTempElement;
	return pRetVal;
}


/*******************************************************************************
** FUNCTION:    Sdf_cl_commonHash:: forEach
**
** DESCRIPTION: This method iterates through the entire hash table, pointing 
**				to every element in the hash table. Every time this method is 
**				called it points to the next element.
*******************************************************************************/
void Sdf_cl_commonHash :: forEach\
	(\
	Sdf_ty_hashIteratorFunc fpIteratorFunc\
	)
{
	Sdf_ty_U32bit i;
	Sdf_st_commonHashElement *pIterator = Sdf_co_null;
	/* Iterate through all buckets. */
	for (i = 0; i < this->numberOfBuckets; i++)
	{
		/* Iterate through elements in the bucket. */
		pIterator = this->ppHashChains[i];
		while (Sdf_co_null != pIterator)
		{
			if (Sdf_co_fail == fpIteratorFunc(pIterator->pKey,\
					pIterator->pElement))
				return;
			pIterator = pIterator->pNext;
		}
	}
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonHash:: next
**
** DESCRIPTION: This method makes the iterator point to the next element in 
**				the hash table.
*******************************************************************************/
void Sdf_cl_commonHash :: next\
	(\
	Sdf_st_commonHashIterator *pIterator\
	)
{
	/* If current element in the iterator points to a null node -
	 * keep it null else make it point to next element in the chain. */
	pIterator->pCurrentElement = (Sdf_co_null == pIterator->pCurrentElement)\
		? Sdf_co_null : pIterator->pCurrentElement->pNext;

	if (Sdf_co_null != pIterator->pCurrentElement)
		return;
	/* If the iterator is pointing to a null node in the last chain, return. */
	if ((Sdf_co_null == pIterator->pCurrentElement) &&\
		(pIterator->currentBucket == (this->numberOfBuckets - 1)))
		return;
	/* Find the next non-empty chain and make the iterator point to that. */
	pIterator->currentBucket++;
	while ((pIterator->currentBucket != (this->numberOfBuckets - 1)) &&\
		(Sdf_co_null == this->ppHashChains[pIterator->currentBucket]))
		pIterator->currentBucket++;
	pIterator->pCurrentElement = this->ppHashChains[pIterator->currentBucket];
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonHash:: initIterator
**
** DESCRIPTION: This method initializes the iterator.
*******************************************************************************/
void Sdf_cl_commonHash :: initIterator\
	(\
	Sdf_st_commonHashIterator *pIterator\
	)
{
	pIterator->pCurrentElement = this->ppHashChains[0];
	pIterator->currentBucket = 0;
	if (Sdf_co_null == pIterator->pCurrentElement)
		this->next(pIterator);
}	

/*******************************************************************************
** Function     : Sdf_cl_commonHash::initIterator                             **
**                                                                            **
** Description  : Initialises the hash iterator for a particular chain        **
**                                                                            **
** Returns      : The initialised iterator in the parameter "pIterator". The  **
**                element in the iterator may be NULL                         **
**                                                                            **
*******************************************************************************/
void
Sdf_cl_commonHash::initIterator(
    SDF_INOUT   void                            *pKey,
    SDF_INOUT   Sdf_st_commonHashIterator       *pIterator )
{
    // First locate the chain
    Sdf_ty_U32bit hashKey = this->fpHashFunc( pKey );
    Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
    // Now init it
    pIterator->pCurrentElement = this->ppHashChains[bucket];
    pIterator->currentBucket = bucket;
}

/******************************************************************************
 ** Function	: Sdf_cl_commonHash::initIterator							 **
 ** 																		 **
 ** Description	: Initializes the has iterator for a particular chain		 **
 ** 																		 **
 ** Returns		: The initialized iterator in the parameter "pIterator". The **
 **				  element in the iterator may be NULL						 **
 **																			 **
 ******************************************************************************/
 void Sdf_cl_commonHash :: initIterator(
 	SDF_IN		Sdf_ty_U32bit				dCorrId,
	SDF_INOUT	Sdf_st_commonHashIterator	*pIterator)
 {
	// First locate the chain
	Sdf_ty_U32bit bucket = dCorrId % this->numberOfBuckets;
	
	// Now initialize the iterator
	pIterator->pCurrentElement = this->ppHashChains[bucket];
	pIterator->currentBucket = bucket;
 }
 
/*******************************************************************************
** Function     : Sdf_cl_commonHash::nextInChain                              **
**                                                                            **
** Description  : Retrieves the next element in the chain                     **
**                                                                            **
** Returns      : The next element, which may be NULL if there are no more    **
**                elements in the chain                                       **
**                                                                            **
*******************************************************************************/
void 
Sdf_cl_commonHash::nextInChain(
    SDF_INOUT   Sdf_st_commonHashIterator       *pIterator )
{
    pIterator->pCurrentElement = (Sdf_co_null == pIterator->pCurrentElement)?
                                Sdf_co_null:pIterator->pCurrentElement->pNext;
}
/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: Sdf_cl_commonThreadSafeHash
**
** DESCRIPTION: This is the Constructor for the class. Initializes the mutexes 
**				for all the buckets. This method also calls the constructor of 
**				Sdf_cl_commonHash class to initialize the Hash table.
*******************************************************************************/
Sdf_cl_commonThreadSafeHash :: Sdf_cl_commonThreadSafeHash\
	(\
	Sdf_ty_hashFunc 			fpHashFunc,\
	Sdf_ty_hashKeyCompareFunc 	fpKeyCompareFunc,\
	Sdf_ty_hashKeyFreeFunc 		fpKeyFreeFunc,\
	Sdf_ty_hashElemFreeFunc 	fpElemFreeFunc,\
	Sdf_ty_U32bit 				numBuckets, \
	Sdf_ty_bool					enableRecursiveFetch
	)
	:Sdf_cl_commonHash(fpHashFunc, fpKeyCompareFunc, fpKeyFreeFunc,\
		fpElemFreeFunc, numBuckets
		)
{
	this->pBucketLock = new Sdf_ty_mutexT[numBuckets];

	Sdf_ty_mutexAttrT mutexAttr;
	pthread_mutexattr_init(&mutexAttr);
	for (Sdf_ty_U32bit i = 0; i < numBuckets; i++)
	{
		if(Sdf_co_true == enableRecursiveFetch)
		{
			pthread_mutex_init(&(this->pBucketLock[i]), &mutexAttr);
		}
		else
		{
			pthread_mutex_init(&(this->pBucketLock[i]), Sdf_co_null);
		}
	}

	pthread_mutexattr_destroy(&mutexAttr);

	pthread_mutex_init(&(this->hashLock), Sdf_co_null);
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: ~Sdf_cl_commonThreadSafeHash
**
** DESCRIPTION: This is the Destructor for the class. It deletes all the 
**				elements in the hash table. It destroys all the mutexes also.
*******************************************************************************/
Sdf_cl_commonThreadSafeHash :: ~Sdf_cl_commonThreadSafeHash\
	(\
	void\
	)
{
	/* 
	 * Iterate through the entire hash table and remove all the elements. Use
	 * the fpHashElemFreeFunc to free all the elements removed. Finally delete
	 * all the buckets. Delete all the Bucket level locks also.
	 */
	Sdf_ty_error  err;
	
	if (Sdf_co_null != this->fpHashElemFreeFunc)
	{
		Sdf_st_commonHashIterator iterator;
		this->initIterator(&iterator);
		while (Sdf_co_null != iterator.pCurrentElement)
		{
			void* pKey = iterator.pCurrentElement->pKey;
			this->next(&iterator);
			void* pData = this->remove(pKey);
			if (Sdf_co_null != pData)
				this->fpHashElemFreeFunc(pData);
		}
	}
	/* Destroy the mutex before deleting the Bucket level locks. */
	for (Sdf_ty_U32bit i = 0; i < this->numberOfBuckets; i++)
	{
		if (Sdf_co_success != pthread_mutex_destroy(&(this->pBucketLock[i])))
		{
			NULL;
		}
	}
	delete[] (this->pBucketLock);
 
 	/* Destroy the hashLock Mutex. */
    pthread_mutex_destroy(&(this->hashLock));

}


/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash:: xFetchRemove
**
** DESCRIPTION: This method removes the element from the hash table.
*******************************************************************************/
void *Sdf_cl_commonThreadSafeHash:: xFetchRemove\
	(\
	Sdf_ty_U32bit correlationID\
	)
{
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = correlationID % this->numberOfBuckets;

	/* Bucket level locking.Unlocking is done in xRelease method. */
	pthread_mutex_lock(&(this->pBucketLock[bucket]));

	/* Go through chain. */
	Sdf_st_commonHashElement **ppIterator = &(this->ppHashChains[bucket]);
	while (Sdf_co_null != *ppIterator)
	{
		if ((*ppIterator)->correlationID == correlationID)
			break;
		ppIterator = &((*ppIterator)->pNext);
	}
	/* If there is no element to remove from the hash table, return NULL. */
	if (Sdf_co_null == *ppIterator)
		return Sdf_co_null;
	/* Remove the element from the Hash table. */
	void *pRetVal = (*ppIterator)->pElement;
	Sdf_st_commonHashElement *pTempElement = *ppIterator;
	*ppIterator = (*ppIterator)->pNext;
	if (Sdf_co_null != this->fpHashKeyFreeFunc)
	{
		this->fpHashKeyFreeFunc(pTempElement->pKey);
	}
	/* decrement the total number of elements in the hash table. */
	
	/* hash table level locking. */
	pthread_mutex_lock(&(this->hashLock));
	(this->numberOfElements)--;
	/* hash table level unlocking. */
	pthread_mutex_unlock(&(this->hashLock));

	delete pTempElement;
	return pRetVal;
}


/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: size
**
** DESCRIPTION: This method returns the number of elements in the hash table.
*******************************************************************************/
Sdf_ty_S32bit Sdf_cl_commonThreadSafeHash :: size\
	(\
	void\
	)
{
	return this->numberOfElements;
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: add
**
** DESCRIPTION: This method is used to add elements to the hash table.
*******************************************************************************/
void Sdf_cl_commonThreadSafeHash :: add\
	(\
	void 			*pElement,\
	void 			*pKey, \
	Sdf_ty_U32bit	*pCorrelationID
	)
{

	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = this->fpHashFunc(pKey);
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
	/* Allocate and initialize element holder. */
	Sdf_st_commonHashElement* pNewElement = new Sdf_st_commonHashElement;
	pNewElement->pElement = pElement;
	pNewElement->pKey = pKey;
	/* Initialize reference count of hash element to 1. */
	pNewElement->dRefCount = 1;
	
	/* bucket level locking. */
	pthread_mutex_lock(&(this->pBucketLock[bucket]));

	/* Push element into the bucket. */
	pNewElement->pNext = this->ppHashChains[bucket];
	this->ppHashChains[bucket] = pNewElement;

	if(Sdf_co_null != pCorrelationID)
	{
		Sdf_ty_U32bit corrID=0;

		/*
		 * The correlation ID is generated as per following arithmatic.
		 * correlation ID = (bucketCounter * numberOfBuckets) + bucketIndex
		 *
		 * If correlation ID exceeds Sdf_co_u32Max, then the bucketCounter must
		 * be re-initialized to 1.
		 * 
		 * Therefore, correlation ID <= Sdf_co_u32Max
		 * => (bucketCounter * numberOfBuckets) + bucketIndex <= Sdf_co_u32Max
		 * => (bucketCounter * numberOfBuckets) <= (u32Max - bucketIndex)
		 * => bucketCounter <= ((u32Max - bucketIndex)/numberOfBuckets)
		 * 
		 * Therefore if bucketCounter > ((u32Max - bucketIndex)/numberOfBuckets)
		 * RESET bucketCounter to "1".
		 */
		
		if ((this->bucketCounter[bucket]) > \
			((Sdf_co_u32Max - bucket)/this->numberOfBuckets))
		{
			this->bucketCounter[bucket] = 1;
		}
		corrID = (this->bucketCounter[bucket] * this->numberOfBuckets) + bucket;
		
		/* Increment the corr ID generating counter. */
		(this->bucketCounter[bucket])++;

		/* Add the correlation-ID in the hash element */
		pNewElement->correlationID= corrID;

		/* Return the correlation id */
		*pCorrelationID = corrID;
	}
	
	/* hash table level locking. */
	pthread_mutex_lock(&(this->hashLock));
	/* increment the total number of elements in the hash table. */
	(this->numberOfElements)++;
	/* hash table level unlocking. */
	pthread_mutex_unlock(&(this->hashLock));

	/* bucket level unlocking. */
	pthread_mutex_unlock(&(this->pBucketLock[bucket]));
}

/*******************************************************************************
** Function     : Sdf_cl_commonThreadSafeHash::addUnique                      **
**                                                                            **
** Description  : This method is used to add unique elements to the hash table**
**                                                                            **
*******************************************************************************/
Sdf_ty_retVal Sdf_cl_commonThreadSafeHash::addUnique(
        void                                *pElement,
        void                                *pKey,
        Sdf_ty_error                        *pEcode )
{
    Sdf_ty_retVal                           retval              = Sdf_co_fail;

    // Compute hash for the element
    Sdf_ty_U32bit hashKey = this->fpHashFunc(pKey);
    // Locate the bucket
    Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
    // now check to see if this element has already been inserted in the hash
    // first do a bucket level lock
    pthread_mutex_lock(&(this->pBucketLock[bucket]));
    Sdf_st_commonHashElement *pNextElem = this->ppHashChains[bucket];
    while( Sdf_co_null != pNextElem )
    {
        if( Sdf_co_success == this->fpCompareFunc(pNextElem->pKey, pKey) )
        {
            break;
        }
        pNextElem = pNextElem->pNext;
    }
    if( Sdf_co_null == pNextElem )
    {
        // Element has not been found. Go ahead
        // Allocate and initialize element holder
        Sdf_st_commonHashElement *pNewElement = new Sdf_st_commonHashElement;
        pNewElement->pElement = pElement;
        pNewElement->pKey = pKey;
        // Initialize reference count of hash element to 1
        pNewElement->dRefCount = 1;
        // Push element into the bucket
        pNewElement->pNext = this->ppHashChains[bucket];
        this->ppHashChains[bucket] = pNewElement;
        /* hash table level locking. */
        pthread_mutex_lock(&(this->hashLock));
        //increment the total number of elements in the hash table
        (this->numberOfElements)++;
        // Release the hash level lock
        pthread_mutex_unlock(&(this->hashLock));
        // Release the bucket level lock
        pthread_mutex_unlock(&(this->pBucketLock[bucket]));
        retval = Sdf_co_success;
    }
    else
    {
        // Element already exists. Return  failure
        // first release the lock
        pthread_mutex_unlock(&(this->pBucketLock[bucket]));
        *pEcode = Sdf_en_errorDuplicateData;
    }
    
    return retval;
}


/******************************************************************************
** Function     : Sdf_cl_commonThreadSafeHash::addUnique                      **
**                                                                            **
** Description  : This method is used to add unique elements to the hash table**
**					and resutns a correlation ID 
**                                                                            **
******************************************************************************/
Sdf_ty_retVal Sdf_cl_commonThreadSafeHash::addUnique(
        void                                *pElement,
		void								*pKey,\
		Sdf_ty_U32bit						*pCorrelationID,\
        Sdf_ty_error                        *pEcode )
{
	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = this->fpHashFunc(pKey);
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
    Sdf_ty_retVal	retval= Sdf_co_fail;

	// now check to see if this element has already been inserted in the hash
    // first do a bucket level lock
    pthread_mutex_lock(&(this->pBucketLock[bucket]));
    Sdf_st_commonHashElement *pNextElem = this->ppHashChains[bucket];
    while( Sdf_co_null != pNextElem )
    {
        if( Sdf_co_success == this->fpCompareFunc(pNextElem->pKey, pKey) )
        {
            break;
        }
        pNextElem = pNextElem->pNext;
    }
    if( Sdf_co_null == pNextElem )
    {
        // Element has not been found. Go ahead
        // Allocate and initialize element holder
        Sdf_st_commonHashElement *pNewElement = new Sdf_st_commonHashElement;
        pNewElement->pElement = pElement;
        pNewElement->pKey = pKey;
        // Initialize reference count of hash element to 1
        pNewElement->dRefCount = 1;
		 // Push element into the bucket
        pNewElement->pNext = this->ppHashChains[bucket];

		Sdf_ty_U32bit corrID=0;

		/*
		 * The correlation ID is generated as per following arithmatic.
		 * correlation ID = (bucketCounter * numberOfBuckets) + bucketIndex
		 *
		 * If correlation ID exceeds Sdf_co_u32Max, then the bucketCounter must
		 * be re-initialized to 1.
		 * 
		 * Therefore, correlation ID <= Sdf_co_u32Max
		 * => (bucketCounter * numberOfBuckets) + bucketIndex <= Sdf_co_u32Max
		 * => (bucketCounter * numberOfBuckets) <= (u32Max - bucketIndex)
		 * => bucketCounter <= ((u32Max - bucketIndex)/numberOfBuckets)
		 * 
		 * Therefore if bucketCounter > ((u32Max - bucketIndex)/numberOfBuckets)
		 * RESET bucketCounter to "1".
		 */
		
		if ((this->bucketCounter[bucket]) > \
			((Sdf_co_u32Max - bucket)/this->numberOfBuckets))
		{
			this->bucketCounter[bucket] = 1;
		}
		corrID = (this->bucketCounter[bucket] * this->numberOfBuckets) + bucket;
		
		/* Increment the corr ID generating counter. */
		(this->bucketCounter[bucket])++;

		/* Add the correlation-ID in the hash element */
		pNewElement->correlationID= corrID;

		/* Return the correlation id */
		*pCorrelationID = corrID;

		 // Push element into the bucket
		this->ppHashChains[bucket] = pNewElement;

        /* hash table level locking. */
        pthread_mutex_lock(&(this->hashLock));
        //increment the total number of elements in the hash table
        (this->numberOfElements)++;
        // Release the hash level lock
        pthread_mutex_unlock(&(this->hashLock));
        // Release the bucket level lock
        pthread_mutex_unlock(&(this->pBucketLock[bucket]));
        retval = Sdf_co_success;
    }
    else
    {
        // Element already exists. Return  failure
        // first release the lock
        pthread_mutex_unlock(&(this->pBucketLock[bucket]));
        *pEcode = Sdf_en_errorDuplicateData;
    }

    return retval;
}

/******************************************************************************
 ** Function:	Sdf_cl_commonThreadSafeHash :: add
 ** Description: The other flavours of add interface take a key and generate a 
 ** session id using the the bucket counter. This method takes a correlationId 
 ** and uses that to insert the element into the hash table. Further this
 ** interface will reset the bucket counter based on session id that has
 ** been passed to ensure that the same id does not get generated again
 *****************************************************************************/
void Sdf_cl_commonThreadSafeHash :: add\
	(\
	 Sdf_ty_U32bit		dCorrelationId,\
	 void*				pKey,\
	 void*				pData\
	)
{

	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = 0;
	if(0 != dCorrelationId)
		bucket = dCorrelationId % this->numberOfBuckets;
	else
		bucket = 0;
	/* Allocate and initialize element holder. */
	Sdf_st_commonHashElement* pNewElement = new Sdf_st_commonHashElement;
	pNewElement->pElement = pData;
	pNewElement->pKey = pKey;
	/* Initialize reference count of hash element to 1. */
	pNewElement->dRefCount = 1;
	pNewElement->correlationID = dCorrelationId;
	
	/* bucket level locking. */
	pthread_mutex_lock(&(this->pBucketLock[bucket]));

	/* Push element into the bucket. */
	pNewElement->pNext = this->ppHashChains[bucket];
	this->ppHashChains[bucket] = pNewElement;

	/* The bucket counter value has to be re-computed based on the
	 * correlation that is getting added newly. The bucket counter can be
	 * calculated as follows
	 * tempBucketCounter = (dCorrelationId - bucketIndex) / numberOfBuckets
	 * if(tempBucketCounter >= currentBucketCounter)
	 * {
	 * 		Update the counter value to tempBucketCounter + 1. This will
	 * 		ensure that the correlation id generated by this bucket will
	 * 		not overlap with the ones that have already been added
	 * }
	 * Ensure that the newly assigned bucket counter does not generate a
	 * correlation id greater than or equal to Sdf_co_u32Max. If so reset
	 * the same to 1
	 */

	Sdf_ty_U32bit tempBucketCount = 0;

	tempBucketCount = (dCorrelationId - bucket) / this->numberOfBuckets;

	// If the newly computed counter is not greater than the current bucket
	// counter then the same need not be updated.
	if(tempBucketCount >= this->bucketCounter[bucket])
	{
		this->bucketCounter[bucket] = tempBucketCount + 1;

		if ((this->bucketCounter[bucket]) > \
			((Sdf_co_u32Max - bucket)/this->numberOfBuckets))
		{
			this->bucketCounter[bucket] = 1;
		}
	}
	
	/* hash table level locking. */
	pthread_mutex_lock(&(this->hashLock));
	/* increment the total number of elements in the hash table. */
	(this->numberOfElements)++;
	/* hash table level unlocking. */
	pthread_mutex_unlock(&(this->hashLock));

	/* bucket level unlocking. */
	pthread_mutex_unlock(&(this->pBucketLock[bucket]));
}
 

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: remove
**
** DESCRIPTION: This method decrements the reference count. If the reference 
**				count is zero it removes the element from the hash table. If 
**				the reference count is not zero, it increments the reference 
**				count again.
*******************************************************************************/
void* Sdf_cl_commonThreadSafeHash :: remove\
	(\
	void *pKey\
	)
{

	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = fpHashFunc(pKey);
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
	
	/* bucket level locking. */
	pthread_mutex_lock(&(this->pBucketLock[bucket]));

	/* Go through chain. */
	Sdf_st_commonHashElement **ppIterator = &(this->ppHashChains[bucket]);
	while (Sdf_co_null != *ppIterator)
	{
		if (Sdf_co_success == this->fpCompareFunc((*ppIterator)->pKey, pKey))
			break;
		ppIterator = &((*ppIterator)->pNext);
	}
	if (Sdf_co_null == *ppIterator)
	{
		/* bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[bucket]));
		return Sdf_co_null;
	}

	/* Decrement the reference count of the hash element. */
	((*ppIterator)->dRefCount)--;

	if (0 == (*ppIterator)->dRefCount)
	{
		void *pRetVal = (*ppIterator)->pElement;
		Sdf_st_commonHashElement *pTempElement = *ppIterator;
		*ppIterator = (*ppIterator)->pNext;

		if (Sdf_co_null != this->fpHashKeyFreeFunc)
		{
			this->fpHashKeyFreeFunc(pTempElement->pKey);
		}
		delete pTempElement;
		/* hash table level locking. */
		pthread_mutex_lock(&(this->hashLock));
		/* decrement the total number of elements in the hash table. */
		(this->numberOfElements)--;
		/* hash table level unlocking. */
		pthread_mutex_unlock(&(this->hashLock));
		/* bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[bucket]));
		return pRetVal;
	}
	else
	{
		((*ppIterator)->dRefCount)++;
		/* bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[bucket]));
		return Sdf_co_null;
	}
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: remove
**
** DESCRIPTION: This method decrements the reference count. If the reference 
**				count is zero it removes the element from the hash table. If 
**				the reference count is not zero, it increments the reference 
**				count again. This uses the correlation ID 
*******************************************************************************/
void* Sdf_cl_commonThreadSafeHash :: remove\
	(\
		Sdf_ty_U32bit	correlationID\
	)
{
	Sdf_ty_U32bit bucket = correlationID % this->numberOfBuckets;
					
	pthread_mutex_lock(&(this->pBucketLock[bucket]));

	/* Go thru the elements in the bucket to get the matching element */
	Sdf_st_commonHashElement* pIterator = this->ppHashChains[bucket];
	Sdf_st_commonHashElement* pPrev = Sdf_co_null;
	while (Sdf_co_null != pIterator)
	{
		if (pIterator->correlationID == correlationID)
			break;
        pPrev = pIterator;
		pIterator = pIterator->pNext;
	}	

	if (Sdf_co_null == pIterator)
	{
		/* bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[bucket]));
		return Sdf_co_null;
	}

	/* Decrement the reference count of the hash element. */
	((pIterator)->dRefCount)--;

	if (0 == (pIterator)->dRefCount)
	{
		void *pRetVal = (pIterator)->pElement;
		Sdf_st_commonHashElement *pTempElement = pIterator;
		pIterator = (pIterator)->pNext;

        if (pPrev != Sdf_co_null)
            pPrev->pNext = pIterator;
        else
            this->ppHashChains[bucket] = pIterator;

		if (Sdf_co_null != this->fpHashKeyFreeFunc)
		{
			this->fpHashKeyFreeFunc(pTempElement->pKey);
		}
		delete pTempElement;
		/* hash table level locking. */
		pthread_mutex_lock(&(this->hashLock));
		/* decrement the total number of elements in the hash table. */
		(this->numberOfElements)--;
		/* hash table level unlocking. */
		pthread_mutex_unlock(&(this->hashLock));
		/* bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[bucket]));
		return pRetVal;
	}
	else
	{
		((pIterator)->dRefCount)++;
		/* bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[bucket]));
		return Sdf_co_null;
	}
}


/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: fetch
**
** DESCRIPTION: This method fetches the element from the hash table.
*******************************************************************************/
void* Sdf_cl_commonThreadSafeHash :: fetch\
	(\
	void *pKey\
	)
{

	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = fpHashFunc(pKey);
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;

	/* bucket level locking. */
	pthread_mutex_lock(&(this->pBucketLock[bucket]));

	/* Go through chain. */
	Sdf_st_commonHashElement* pIterator = this->ppHashChains[bucket];
	while (Sdf_co_null != pIterator)
	{
		if (Sdf_co_success == this->fpCompareFunc(pIterator->pKey, pKey))
			break;
		pIterator = pIterator->pNext;
	}
	if (Sdf_co_null != pIterator)
	{
		/* Increment the reference count for the hash element. */
		(pIterator->dRefCount)++;
		/* bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[bucket]));
		return pIterator->pElement;
	}
	else
	{
		/* bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[bucket]));
		return Sdf_co_null;
	}
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: fetch
**
** DESCRIPTION: This method fetches the element from the hash table.
*******************************************************************************/
void* Sdf_cl_commonThreadSafeHash :: fetch\
	(\
	Sdf_ty_U32bit	correlationID\
	)
{
	Sdf_ty_U32bit bucket = correlationID % this->numberOfBuckets;
					
	pthread_mutex_lock(&(this->pBucketLock[bucket]));

	/* Go thru the elements in the bucket to get the matching element */
	Sdf_st_commonHashElement* pIterator = this->ppHashChains[bucket];
	while (Sdf_co_null != pIterator)
	{
		if (pIterator->correlationID == correlationID)
			break;
		pIterator = pIterator->pNext;
	}	
	if (Sdf_co_null != pIterator)
	{
		/* Increment the reference count for the hash element. */
		(pIterator->dRefCount)++;
		/* bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[bucket]));
		return pIterator->pElement;
	}
	else
	{
		/* bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[bucket]));
		return Sdf_co_null;
	}
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: release
**
** DESCRIPTION: This method decrements the reference count for a particular 
**				element and it does not delete the element.
*******************************************************************************/
void Sdf_cl_commonThreadSafeHash :: release\
	(\
	void *pKey\
	)
{

	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = fpHashFunc(pKey);
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
	/* bucket level locking. */
	pthread_mutex_lock(&(this->pBucketLock[bucket]));

	/* Go through chain. */
	Sdf_st_commonHashElement* pIterator = this->ppHashChains[bucket];
	while (Sdf_co_null != pIterator)
	{
		if (Sdf_co_success == this->fpCompareFunc(pIterator->pKey, pKey))
			break;
		pIterator = pIterator->pNext;
	}
	if (Sdf_co_null != pIterator)
	{
		/* Decrement the reference count for the hash element. */
		(pIterator->dRefCount)--;
	}
	/* bucket level unlocking. */
	pthread_mutex_unlock(&(this->pBucketLock[bucket]));
	return;
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: release
**
** DESCRIPTION: This method decrements the reference count for a particular 
**				element and it does not delete the element.
*******************************************************************************/
void Sdf_cl_commonThreadSafeHash :: release\
	(\
	Sdf_ty_U32bit correlationID\
	)
{

	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = correlationID % this->numberOfBuckets;
	/* bucket level locking. */
	pthread_mutex_lock(&(this->pBucketLock[bucket]));

	/* Go through chain. */
	Sdf_st_commonHashElement* pIterator = this->ppHashChains[bucket];
	while (Sdf_co_null != pIterator)
	{
		if (pIterator->correlationID == correlationID)
			break;
		pIterator = pIterator->pNext;
	}
	if (Sdf_co_null != pIterator)
	{
		/* Decrement the reference count for the hash element. */
		(pIterator->dRefCount)--;
	}
	/* bucket level unlocking. */
	pthread_mutex_unlock(&(this->pBucketLock[bucket]));
	return;
}




/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: forEach
**
** DESCRIPTION: This method iterates through the entire hash table, pointing 
**				to every element in the hash table. Every time this method is 
**				called it points to the next element.
*******************************************************************************/
void Sdf_cl_commonThreadSafeHash :: forEach\
	(\
	Sdf_ty_hashIteratorFunc fpIteratorFunc\
	)
{
	Sdf_ty_U32bit i;
	/* Iterate through all buckets. */
	for (i = 0; i < this->numberOfBuckets; i++)
	{
		/* Iterate through elements in the bucket. Bucket level locking. */
		pthread_mutex_lock(&(this->pBucketLock[i]));

		Sdf_st_commonHashElement *pIterator = this->ppHashChains[i];
		while (Sdf_co_null != pIterator)
		{
			if (Sdf_co_fail == fpIteratorFunc(pIterator->pKey,\
				pIterator->pElement))
			{
				/* Bucket level unlocking. */
				pthread_mutex_unlock(&(this->pBucketLock[i]));
				return;
			}
			pIterator = pIterator->pNext;
		}
		/* Bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[i]));

	}
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: initIterator
**
** DESCRIPTION: This method initializes the iterator.
*******************************************************************************/
void Sdf_cl_commonThreadSafeHash :: initIterator\
	(\
	Sdf_st_commonHashIterator *pIterator\
	)
{
	pIterator->pCurrentElement = this->ppHashChains[0];
	pIterator->currentBucket = 0;
	if (Sdf_co_null == pIterator->pCurrentElement)
		this->next(pIterator);
	else
		pIterator->pCurrentElement->dRefCount++;
}

/*******************************************************************************
** Function     : Sdf_cl_commonThreadSafeHash::initIterator                   **
**                                                                            **
** Description  : Initialises the hash iterator for a particular chain        **
**                                                                            **
** Returns      : The initialised iterator in the parameter "pIterator". The  **
**                element in the iterator may be NULL                         **
**                                                                            **
*******************************************************************************/
void
Sdf_cl_commonThreadSafeHash::initIterator(
    SDF_INOUT   void                            *pKey,
    SDF_INOUT   Sdf_st_commonHashIterator       *pIterator )
{
    // First locate the chain
    Sdf_ty_U32bit hashKey = this->fpHashFunc( pKey );
    Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
    // Now init it
    pIterator->pCurrentElement = this->ppHashChains[bucket];
    pIterator->currentBucket = bucket;
    if( Sdf_co_null != pIterator->pCurrentElement )
    {
        pIterator->pCurrentElement->dRefCount++;
    }
}

/*******************************************************************************
** Function     : Sdf_cl_commonThreadSafeHash::initIterator                   **
**                                                                            **
** Description  : Initialises the hash iterator for a particular chain based  **
**				  on the correlation id that has been passed				  **
**                                                                            **
** Returns      : The initialised iterator in the parameter "pIterator". The  **
**                element in the iterator may be NULL                         **
**                                                                            **
*******************************************************************************/
void
Sdf_cl_commonThreadSafeHash::initIterator(
    SDF_IN	    Sdf_ty_U32bit					dCorrId,
    SDF_INOUT   Sdf_st_commonHashIterator       *pIterator )
{
    // First locate the chain
    Sdf_ty_U32bit bucket =dCorrId % this->numberOfBuckets;
    // Now init it
    pIterator->pCurrentElement = this->ppHashChains[bucket];
    pIterator->currentBucket = bucket;
    if( Sdf_co_null != pIterator->pCurrentElement )
    {
        pIterator->pCurrentElement->dRefCount++;
    }
}

/*****************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: xInitIterator
**
** DESCRIPTION: This method initializes the iterator.While initializing 
** it locks the bucket.
*****************************************************************************/
void Sdf_cl_commonThreadSafeHash :: xInitIterator\
	(\
	Sdf_st_commonHashIterator *pIterator\
	)
{
	/* NOTE : In case application code wants remove any emenent of the hash
	table while iterating with xInitIterator then it must use base class
	API i.e. Sdf_cl_commonHash :: remove */

	pIterator->pCurrentElement = this->ppHashChains[0];
	pIterator->currentBucket = 0;
	pthread_mutex_lock(&(this->pBucketLock[pIterator->currentBucket]));
	if (Sdf_co_null == pIterator->pCurrentElement)
		this->xNext(pIterator);
}

/****************************************************************************
** Function     : Sdf_cl_commonThreadSafeHash::xInitIterator               **
**                                                                         **
** Description  : Initialises the hash iterator for a particular chain and **
** 				  locks the chain                                          **
****************************************************************************/
void
Sdf_cl_commonThreadSafeHash::xInitIterator(
    SDF_INOUT   void                            *pKey,
    SDF_INOUT   Sdf_st_commonHashIterator       *pIterator )
{
    // First locate the chain
    Sdf_ty_U32bit hashKey = this->fpHashFunc( pKey );
    Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
    // Now initialise it
    pIterator->pCurrentElement = this->ppHashChains[bucket];
    pIterator->currentBucket = bucket;
    if( Sdf_co_null != pIterator->pCurrentElement )
    {
		pthread_mutex_lock(&(this->pBucketLock[pIterator->currentBucket]));
    }
}

/****************************************************************************
** Function     : Sdf_cl_commonThreadSafeHash::xInitIterator               **
**                                                                         **
** Description  : Initialises the hash iterator for a particular chain and **
** 				  locks the chain                                          **
****************************************************************************/
void
Sdf_cl_commonThreadSafeHash::xInitIterator(
    /*SDF_INOUT*/SDF_IN   Sdf_ty_U32bit                   dCorrId,
    SDF_INOUT   Sdf_st_commonHashIterator       *pIterator )
{
    // First locate the chain
    Sdf_ty_U32bit bucket = dCorrId % this->numberOfBuckets;
    // Now initialise it
    pIterator->pCurrentElement = this->ppHashChains[bucket];
    pIterator->currentBucket = bucket;
    if( Sdf_co_null != pIterator->pCurrentElement )
    {
		pthread_mutex_lock(&(this->pBucketLock[pIterator->currentBucket]));
    }
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: completeXIteration 
**
** DESCRIPTION: This method unlocks the chain if we break out of the 
** 				iteration in the middle.
*******************************************************************************/
void Sdf_cl_commonThreadSafeHash :: completeXIteration\
	(\
	Sdf_st_commonHashIterator *pIterator\
	)
{
	
	if (Sdf_co_null == pIterator->pCurrentElement)
		return;
	pthread_mutex_unlock(&(this->pBucketLock[pIterator->currentBucket]));
}


/*******************************************************************************
** Function     : Sdf_cl_commonThreadSafeHash::nextInChain                    **
**                                                                            **
** Description  : Retrieves the next element in the chain                     **
**                                                                            **
** Returns      : The next element, which may be NULL if there are no more    **
**                elements in the chain                                       **
**                                                                            **
*******************************************************************************/
void 
Sdf_cl_commonThreadSafeHash::nextInChain(
    SDF_INOUT   Sdf_st_commonHashIterator       *pIterator )
{

    // Lock the bucket
	pthread_mutex_lock( &(this->pBucketLock[pIterator->currentBucket]));

    if( Sdf_co_null != pIterator->pCurrentElement )
    {
        pIterator->pCurrentElement->dRefCount--;
        pIterator->pCurrentElement = pIterator->pCurrentElement->pNext;
    }
    if( Sdf_co_null != pIterator->pCurrentElement )
    {
        pIterator->pCurrentElement->dRefCount++;
    }
    pthread_mutex_unlock( &(this->pBucketLock[pIterator->currentBucket]));

    return;
}
/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: next
**
** DESCRIPTION: This method makes the iterator point to the next element in 
**				the hash table.
*******************************************************************************/
void Sdf_cl_commonThreadSafeHash :: next\
	(\
	Sdf_st_commonHashIterator *pIterator\
	)
{

	/* Bucket level locking. */
	pthread_mutex_lock(&(this->pBucketLock[pIterator->currentBucket]));

	/* If current element is not pointing to NULL then release the refount. */
	if (Sdf_co_null != pIterator->pCurrentElement)
		pIterator->pCurrentElement->dRefCount--;

	/* 
	 * If current element in the iterator points to NULL node keep it
	 * NULL otherwise make it point to the next element in the chain.
	 */
	pIterator->pCurrentElement = (Sdf_co_null == pIterator->pCurrentElement) ?\
		Sdf_co_null : pIterator->pCurrentElement->pNext;

	if (Sdf_co_null != pIterator->pCurrentElement)
	{
		/* increment the refcount of element and unlock bucket. */
		pIterator->pCurrentElement->dRefCount++;
		pthread_mutex_unlock(&(this->pBucketLock[pIterator->currentBucket]));
		return;
	}
	/* If the current element is pointing to NULL in the last chain, return.*/
	if ((Sdf_co_null == pIterator->pCurrentElement) && \
		(pIterator->currentBucket == this->numberOfBuckets - 1))
	{
		/* Bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[pIterator->currentBucket]));
		return;
	}
	/* Bucket level unlocking. */
	pthread_mutex_unlock(&(this->pBucketLock[pIterator->currentBucket]));
	
	/* Find the next non-empty chain and point the iterator to that. */
	pIterator->currentBucket++;
	while (pIterator->currentBucket <= this->numberOfBuckets-1)
	{
		/* Bucket level locking. */
		pthread_mutex_lock(&(this->pBucketLock[pIterator->currentBucket]));
		if (Sdf_co_null != this->ppHashChains[pIterator->currentBucket])
		{
			/* Point the iterator to the first element of the bucket. */
			pIterator->pCurrentElement = \
				this->ppHashChains[pIterator->currentBucket];
				
			pIterator->pCurrentElement->dRefCount++;
			/* Bucket level unlocking. */
			pthread_mutex_unlock(&(this->pBucketLock[pIterator->currentBucket]));
			break;
		}
		/* Bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[pIterator->currentBucket]));
		pIterator->currentBucket++;
	}
	if (pIterator->currentBucket == this->numberOfBuckets)
		pIterator->currentBucket--;
}


/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: xNext
**
** DESCRIPTION: This method makes the xIterator point to the next element in 
**				the hash table.It will not release the lock till all the
**              elements in that bucket are traversed
*******************************************************************************/
void Sdf_cl_commonThreadSafeHash :: xNext\
	(\
	Sdf_st_commonHashIterator *pIterator\
	)
{
	
	/* 
	 * If current element in the iterator points to NULL node keep it
	 * NULL otherwise make it point to the next element in the chain.
	 */
	pIterator->pCurrentElement = (Sdf_co_null == pIterator->pCurrentElement) ?\
		Sdf_co_null : pIterator->pCurrentElement->pNext;

	if (Sdf_co_null != pIterator->pCurrentElement)
		return;
	else
		pthread_mutex_unlock(&(this->pBucketLock[pIterator->currentBucket]));
	
	/* Find the next non-empty chain and point the xIterator to that. */
	pIterator->currentBucket++;
	while (pIterator->currentBucket <= this->numberOfBuckets-1)
	{
		/* Bucket level locking. */
		pthread_mutex_lock(&(this->pBucketLock[pIterator->currentBucket]));
		if (Sdf_co_null != this->ppHashChains[pIterator->currentBucket])
		{

			/* Point the xIterator to the first element of the bucket. */
			pIterator->pCurrentElement = \
				this->ppHashChains[pIterator->currentBucket];
			break;
		}
		/* Bucket level unlocking. */
		pthread_mutex_unlock(&(this->pBucketLock[pIterator->currentBucket]));
		pIterator->currentBucket++;
	}
	if (pIterator->currentBucket == this->numberOfBuckets)
		pIterator->currentBucket--;
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: xFetch
**
** DESCRIPTION: his function is similar to fetch function.But here,the bucket
**				remain locked when the function returns.In this way, the 
**				module which is using this function will get exclusive access 
**				to the bucket data elements until it is released through 
**				another API. Here refCount is not incremented as this is an 
**				exclusive fetch.
*******************************************************************************/
void* Sdf_cl_commonThreadSafeHash ::xFetch\
	(\
	void* pKey\
	)
{

	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = fpHashFunc(pKey);
	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;

	/* Bucket level locking.Unlocking is done in xRelease method. */
	pthread_mutex_lock(&(this->pBucketLock[bucket]));

	Sdf_st_commonHashElement* pIterator = this->ppHashChains[bucket];
	/* Go through chain. */
	while (Sdf_co_null != pIterator)
	{
		if (Sdf_co_success == this->fpCompareFunc(pIterator->pKey, pKey))
			break;
		pIterator = pIterator->pNext;
	}
	if (Sdf_co_null != pIterator)
	{
		return pIterator->pElement;
	}
	else
	{
		return Sdf_co_null;
	}
}


/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: xFetch
**
** DESCRIPTION: his function is similar to fetch function.But here,the bucket
**				remain locked when the function returns.In this way, the 
**				module which is using this function will get exclusive access 
**				to the bucket data elements until it is released through 
**				another API. Here refCount is not incremented as this is an 
**				exclusive fetch.
*******************************************************************************/
void* Sdf_cl_commonThreadSafeHash ::xFetch\
	(\
	Sdf_ty_U32bit correlationID\
	)
{

	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = correlationID % this->numberOfBuckets;

	/* Bucket level locking.Unlocking is done in xRelease method. */
	pthread_mutex_lock(&(this->pBucketLock[bucket]));

	Sdf_st_commonHashElement* pIterator = this->ppHashChains[bucket];
	/* Go through chain. */
	while (Sdf_co_null != pIterator)
	{
		if (pIterator->correlationID == correlationID)
			break;
		pIterator = pIterator->pNext;
	}
	if (Sdf_co_null != pIterator)
	{
		return pIterator->pElement;
	}
	else
	{
		return Sdf_co_null;
	}
}



/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: xRelease
**
** DESCRIPTION: This function is similar to release function.This function is
**				invoked only after a xFetch is done on a key.This method must
**				ensure that the lock is released before the function returns.
**				Here refCount is not decremented.
*******************************************************************************/
void Sdf_cl_commonThreadSafeHash ::xRelease\
	(\
	void* pKey\
	)
{

	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = fpHashFunc(pKey);
	// Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;
	/* Unlocking the mutex which was acquaird in xFetch mathod. */
	pthread_mutex_unlock(&(this->pBucketLock[bucket]));
	return;
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash :: xRelease
**
** DESCRIPTION: This function is similar to release function.This function is
**				invoked only after a xFetch is done on a key.This method must
**				ensure that the lock is released before the function returns.
**				Here refCount is not decremented.
*******************************************************************************/
void Sdf_cl_commonThreadSafeHash ::xRelease\
	(\
	Sdf_ty_U32bit correlationID\
	)
{

	// Locate the bucket. */
	Sdf_ty_U32bit bucket = correlationID % this->numberOfBuckets;
	/* Unlocking the mutex which was acquaird in xFetch mathod. */
	pthread_mutex_unlock(&(this->pBucketLock[bucket]));
	return;
}

/*******************************************************************************
** FUNCTION:    Sdf_cl_commonThreadSafeHash:: addS
**
** DESCRIPTION: This method is used to add elements to the hash table. 
**				The addition is done to the end of the list where the
**				add interface prepends the new hash elements.
**
*******************************************************************************/
void Sdf_cl_commonThreadSafeHash :: addS\
	(\
	void 	*pElement,\
	void	*pKey\
	)
{

	/* Compute hash for the element. */
	Sdf_ty_U32bit hashKey = this->fpHashFunc(pKey);

	/* Locate the bucket. */
	Sdf_ty_U32bit bucket = hashKey % this->numberOfBuckets;

	/* Allocate and initialize element holder. */
	Sdf_st_commonHashElement* pNewElement = new Sdf_st_commonHashElement;
	pNewElement->pElement = pElement;
	pNewElement->pNext = Sdf_co_null;
	pNewElement->dRefCount = 1;
	pNewElement->pKey = pKey;

	/* hash table level locking. */
	pthread_mutex_lock(&(this->hashLock));

	/* increment the total number of elements in the hash table. */
	(this->numberOfElements)++;

	/* hash table level unlocking. */
	pthread_mutex_unlock(&(this->hashLock));

	/* bucket level locking. */
	pthread_mutex_lock(&(this->pBucketLock[bucket]));
	
	/*
	 * Check whether the element being added is the first element.
	 * In this case the value of the pNext of the bucket should point
	 * to the element being added.
	 */
	if(this->ppHashChains[bucket] == Sdf_co_null)
	{
		this->ppHashChains[bucket] = pNewElement;
	}
	else
	{
		/* Traverse through the list and add the new element to the end */
		Sdf_st_commonHashElement *pIterator = this->ppHashChains[bucket];
		while(pIterator->pNext != Sdf_co_null)
			pIterator = pIterator->pNext;

		pIterator->pNext = pNewElement;
	}

	/* bucket level unlocking. */
	pthread_mutex_unlock(&(this->pBucketLock[bucket]));

	return;
}


/*Following is test code*/

typedef struct
{
	Sdf_ty_U32bit a;
}S_data;

static void callHashKeyFree(Sdf_ty_Pvoid key)
{
	delete [] (Sdf_ty_U8bit*)key;
}

static Sdf_ty_U32bit Hash\
	(\
	Sdf_ty_Pvoid *pInputName\
	)
{
    Sdf_ty_U32bit h = 0, g;
	Sdf_ty_U8bit *pName = (Sdf_ty_U8bit*)pInputName;
	
    while (*pName)
    {
        h = (h << 4) + *pName++;
        if ((g = h & 0xf0000000))
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

static Sdf_ty_retVal callHashKeyCompare (Sdf_ty_Pvoid pKey1, Sdf_ty_Pvoid pKey2)
{
	if ( strcmp((Sdf_ty_S8bit *)pKey1,(Sdf_ty_S8bit *)pKey2) == 0 )
	{
		return Sdf_co_success;
	}
	else
	{
		return Sdf_co_fail;
	}
}

static Sdf_ty_U32bit callHashCalculate(
				Sdf_ty_Pvoid a_pKey
				)
{
	Sdf_ty_S8bit *pTempKey  = strdup((Sdf_ty_S8bit *)(a_pKey));
	Sdf_ty_U32bit hashValue = 0;

	// The key is a char string, dup it and elf Hash
	hashValue = Hash((Sdf_ty_Pvoid *)pTempKey);

	delete [] pTempKey;
	return hashValue;
}

static void callHashElementFree(Sdf_ty_Pvoid data)
{
	delete (S_data*)data;
}



