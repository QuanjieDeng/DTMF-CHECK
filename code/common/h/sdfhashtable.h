#ifndef _SDF_HASHTABLE_H_
#define _SDF_HASHTABLE_H_

#include "common.h"

// -----------------------------------------------------------------------------
// TYPE DEFINITIONS 
// Sdf_ty_hashFunc 			- 	Hash function that takes a void pointer and 
// 								returns	a 32 bit hash value.
// Sdf_ty_hashKeyCompareFunc-	Function used to compare hash keys. Function
//								returns Sdf_co_success if keys are equal.
// Sdf_ty_hashIteratorFunc 	-	Function pointer to be passed by the user to the
// 								forEach method for iterating through the hash
// 								table entries.
// Sdf_ty_hashKeyFreeFunc	-	Function used to de-allocate keys.
// Sdf_ty_hashElemFreeFunc	-	Function used to de-allocate data stored in 
// 								hash. This call-back is used only in destructor
// 								of the hash table.
// -----------------------------------------------------------------------------
typedef Sdf_ty_U32bit (*Sdf_ty_hashFunc) (void *pData);
typedef Sdf_ty_retVal (*Sdf_ty_hashKeyCompareFunc) (void *pKey1, void *pKey2);

typedef Sdf_ty_retVal (*Sdf_ty_hashIteratorFunc) (void *pKey, void *pElement);
typedef void (*Sdf_ty_hashKeyFreeFunc) (void *pKey);
typedef void (*Sdf_ty_hashElemFreeFunc) (void *pData);

typedef struct Sdf_tg_hashElement
{
	// -------------------------------------------------------------------------
	// DESCRIPTION:	Each node in the hash table.
	// 
	// pElement		-	Element to be stored in the hash table.
	// pKey			-	Key for locating the bucket of the hash table.
	// pNext		- 	Pointer to the next element of the hash table.
	// dRefCount	-	Reference count of the element in the hash table.
	// -------------------------------------------------------------------------
	void*						pElement;
	void*						pKey;
	Sdf_ty_U32bit				correlationID;
	struct Sdf_tg_hashElement*	pNext;
	Sdf_ty_U32bit 				dRefCount;

	Sdf_tg_hashElement(
		)
	{
		this->pElement      = Sdf_co_null;
		this->pKey          = Sdf_co_null;
		this->correlationID = 0;
		this->pNext         = Sdf_co_null;
		this->dRefCount     = 0;
	}
	// Memory Allocators declaration.
	// NOTE: This should be the last declaration in the class.

} Sdf_st_commonHashElement;

typedef struct Sdf_tg_commonHashIterator
{
	// -------------------------------------------------------------------------
	// DESCRIPTION:	For iterating through the hash table.
	// 
	// pCurrentElement	-	Pointer to the current element during iterating.
	// currentBucket	-	The current bucket during iterating.
	// -------------------------------------------------------------------------
	struct Sdf_tg_hashElement 	*pCurrentElement;
	Sdf_ty_U32bit				currentBucket;

	Sdf_tg_commonHashIterator(
		)
	{
		currentBucket = 0;
		pCurrentElement = Sdf_co_null;
	}

} Sdf_st_commonHashIterator;

// -----------------------------------------------------------------------------
// The Hash Table class.
// Note:
// 		Resizing based on load has not been implemented yet.
// -----------------------------------------------------------------------------
class Sdf_cl_commonHash
{
public:
	// -------------------------------------------------------------------------
	// Variable to store the total number of elements in the hash 
	// at any point of time.
	// -------------------------------------------------------------------------
	Sdf_ty_U32bit		numberOfElements;

	// -------------------------------------------------------------------------
	// Constructor: Creates the hash table. Since the hash table stores data
	// as void pointers and is not aware of the type of the data, the user has
	// to provide functions using which the class can generate hashes for the
	// data and for comparing the data.
	// 
	// Parameters are :
	// fpHashFunc:
	// 		The hash function to be used. This is a function provided by the 
	// 		user. This function takes in the data as a void pointer and  
	// 		returns a number based on which the hash map assigns the bucket 
	// 		for the element.
	// fpKeyCompareFunc:
	// 		This is a user provided function. This function is used to compare
	// 		elements in the hash table.
	// fpKeyFreeFunc:
	// 		This user provided function is invoked on the key when an element
	// 		is removed from the hash.
	// fpElemFreeFunc:
	// 		This user provided function is invoked on the element when it is to
	// 		be freed.
	// numberOfBuckets:
	// 		The number of buckets in the hash.
	// -------------------------------------------------------------------------
	Sdf_cl_commonHash\
		(\
		Sdf_ty_hashFunc 			fpHashFunc,\
		Sdf_ty_hashKeyCompareFunc 	fpKeyCompareFunc, \
		Sdf_ty_hashKeyFreeFunc 		fpKeyFreeFunc, \
		Sdf_ty_hashElemFreeFunc 	fpElemFreeFunc, \
		Sdf_ty_U32bit 				numBuckets 	= 5
		);

	// -------------------------------------------------------------------------
	// 	Destructor: Deletes all elements in the hash table.
	// 	
	// 	Parameters are:
	// 	None.
	// -------------------------------------------------------------------------
	~Sdf_cl_commonHash(void);

	// -------------------------------------------------------------------------
	// This method add an element into the hash. The key is passed to the 
	// hash function to determine the bucket in which the element will be
	// stored. The key passed here is stored in the hash till the element is 
	// removed. The key is deleted by invoking the user supplied key free
	// function when the element is removed from the hash.
	// 
	// Parameters are :
	// pElement:
	// 		Element to be added.
	// pKey:
	// 		Key to be used for locating the bucket.
	// -------------------------------------------------------------------------
	void add\
		(\
		void *pElement,\
		void *pKey\
		);

	// -------------------------------------------------------------------------
	// This method add an element into the hash. The key is passed to the 
	// hash function to determine the bucket in which the element will be
	// stored. The key passed here is stored in the hash till the element is 
	// removed. Also, a correlationID is generated.This is stored in the
	// hash element. 
	// The key is deleted by invoking the user supplied key free
	// function when the element is removed from the hash.
	// 
	// Parameters are :
	// pElement:
	// 		Element to be added.
	// pKey:
	// 		Key to be used for locating the bucket.
	// pCorrelationID:
	// 		The correlationID is returned in pCorrelationID .
	// -------------------------------------------------------------------------
	void add\
		(\
		void *pElement,\
		void *pKey,\
		Sdf_ty_U32bit	*pCorrelationID\
		);

	//-------------------------------------------------------------------------
	// Function:	Sdf_cl_commonHash :: add
	//
	// Description: The other flavours of add interface take a key and 
	// generate a session id using the the bucket counter. This method takes a 
	// correlationId and uses that to insert the element into the hash table. 
	// Further this interface will reset the bucket counter based on session 
	// id that has been passed to ensure that the same id does not get 
	// generated again
	//
	// Parameters are:
	// dCorrelationId			- The correlationId using which the data
	// 							has to be inserted into the hash table
	// 
	// pKey						- Any element has two keys. One is the
	// 							correlation id that is maintained within the 
	// 							hash table and the other is a data specific 
	// 							key for which the hashing fn, comparison fn 
	// 							etc are registered by the module using the 
	// 							hash table. The pKey is the module specific key
	// 
	// pData					- The data that has to be stored in the
	// 							hash table
	//-------------------------------------------------------------------------
	void add\
	(\
	 Sdf_ty_U32bit		dCorrelationId,\
	 void*				pKey,\
	 void*				pData\
	);

	// ----------------------------------------------------------------------
	// This method adds the new element to the end of the bucket. The add 
	// interface intirn adds the new element as the first element in the
	// bucket. This method has been added to handle the spiralling cases.
	// ----------------------------------------------------------------------
	void addS\
		(\
		void *pElement,\
		void *pKey\
		);

	// ----------------------------------------------------------------------
	// This method adds the new element to the end of the bucket. The add 
	// interface intirn adds the new element as the first element in the
	// bucket. This method has been added to handle the spiralling cases.
	// ----------------------------------------------------------------------
	void addS\
		(\
		void *pElement,\
		void *pKey,\
		Sdf_ty_U32bit	*pCorrelationID\
		);
    //-------------------------------------------------------------------------
    // This method adds a unique element into the hash table. Operation of
    // this method is simillar to the operation of the 'add()' method, except
    // that the data being added is validate as being unique (ie., no previous
    // instance of the data must exist), before the addition is made to the
    // hash table
    //
    // Returns :
    // Sdf_co_success:
    //      If the adition to the hash table was successful
    // Sdf_co_fail:
    //      If the data could not be added. The reason is indicated by the
    //      pEcode parameter
    // Parameters are :
	// pElement:
	// 		Element to be added.
	// pKey:
	// 		Key to be used for locating the bucket.
    // pEcode:
    //      Return parameter that indicates the reason for failure. Values are:
    //      Sdf_en_errorDuplicateData   - The data could not be added since
    //      the data already exists
    //-------------------------------------------------------------------------
    Sdf_ty_retVal addUnique(
            void                                *pElement,
            void                                *pKey,
			Sdf_ty_U32bit						*pCorrelationID,
            Sdf_ty_error                        *pEcode );
	
    //-------------------------------------------------------------------------
    // This method adds a unique element into the hash table. Operation of
    // this method is simillar to the one above, except that this does not
	// return the correlation ID
	//------------------------------------------------------------------------- 
    Sdf_ty_retVal addUnique(
            void                                *pElement,
            void                                *pKey,
            Sdf_ty_error                        *pEcode );

      Sdf_ty_retVal addUniqueunlock(
            void                                *pElement,
            void                                *pKey,
            Sdf_ty_error                        *pEcode );			
	// -------------------------------------------------------------------------
	// This method retrieves and element from the hash. The key is passed to the
	// hash function to obtain the bucket to be searched. Keys of all elements
	// in the selected bucket are then searched using the user supplied  key 
	// compare function until a match is found. The returned value points to 
	// the element. Returns Sdf_co_null if the the key does not match any key 
	// in the hash. The element and the key (passed in add) remain in the hash
	// table.
	//
	// Parameters are :
	// pKey:
	// 		Key to be used for locating the bucket.
	// -------------------------------------------------------------------------
	void* fetch\
		(\
		void *pKey\
		);
	

	// -------------------------------------------------------------------------
	//  An overload of the fetch method declared above
	//  Used to fetch the element using correlation ID
	// Parameters are :
	// correlationID:
	// 		Key to be used for locating the bucket and the element. 
	// -------------------------------------------------------------------------
	void* fetch\
		(\
		Sdf_ty_U32bit correlationID\
		);


	// -------------------------------------------------------------------------
	// This method extracts and removes an element from the table. The element
	// is found using the user supplied hash and key function. The key stored
	// in the table is released using the user supplied key free function 
	// passed in the constructor. 
	// 
	// Parameters are :
	// pKey:
	// 		Key to be used for locating the bucket.
	// -------------------------------------------------------------------------
	void* remove\
		(\
		void *pKey\
		);

	// -------------------------------------------------------------------------
	//  An overload of the fetch method declared above
	//  Used to fetch the element using correlation ID
	// Parameters are :
	// correlationID:
	// 		Key to be used for locating the bucket and the element. 
	// -------------------------------------------------------------------------
	void* remove\
		(\
		Sdf_ty_U32bit correlationID\
		);
	
	// -------------------------------------------------------------------------	// Function to iterate through the entire table. User passes pointer to 
	// function of type, Sdf_ty_retVal function(void *pKey, void *pElement).
	// Passed function is invoked for each key and element in the list.
	// If the supplied function returns Sdf_co_success the iteration continues 
	// to the next element. Returning Sdf_co_fail stops the iteration.
	//
	// Parameters are :
	// fpIteratorFunc:
	// 		The iterator function.
	// -------------------------------------------------------------------------
	void forEach\
		(\
		Sdf_ty_hashIteratorFunc fpIteratorFunc\
		);
	
	// -------------------------------------------------------------------------	// This method initializes a hash iterator structure to point to the first
	// element in the hash table.The data element pointed by the iterator is
	// accessible as, pIterator->pCurrentElement->pElement and the 
	// corresponding key is accessible as, pIterator->pCurrentElement->pKey.
	//
	// Parameters are :
	// pIterator:
	// 		Pointer to the hash iterator structure.
	// -------------------------------------------------------------------------
	void initIterator\
		(\
		Sdf_st_commonHashIterator *pIterator\
		);
	
    // Initialises the hash iterator to point to the first bucket in a chain
    // identified by the key. Operation is simillar to the above member function
    void initIterator(
        SDF_INOUT   void                        *pKey,
        SDF_INOUT   Sdf_st_commonHashIterator   *pIterator );

    // Initializes the hash iterator to point to the first bucket in a chain
    // identified by the correlation id. Operation is simillar to the above 
	// member function
	void initIterator(
		SDF_IN		Sdf_ty_U32bit				dCorrelationId, \
		SDF_INOUT	Sdf_st_commonHashIterator	*pIterator);

	// -------------------------------------------------------------------------	// Makes the iterator point to the available node in the hash table. The 
	// element and the key are accessible as detailed in the description of
	// initIterator. Using the iteration routines when elements are removed 
	// from the hash gives unpredictable results. On crossing the last element,
	// pIterator->pElement is set to Sdf_co_null.
	//
	// Parameters are :
	// pIterator:
	// 		Pointer to the hash iterator structure.
	// -------------------------------------------------------------------------
	void next\
		(\
		Sdf_st_commonHashIterator *pIterator\
		);
	
    // Bumps the iterato to point to the next bucket in the chain. Movement is 
    // restricted to the current chain unlike the above member function. On
    // encountering the last bucket, subsequent calls return Sdf_co_null
    void nextInChain(
        SDF_INOUT   Sdf_st_commonHashIterator   *pIterator );
	// -------------------------------------------------------------------------	// Fixes the size of the table. The table will not be resized if
	// the load goes out of the acceptable range.
	// -------------------------------------------------------------------------
	// void fixSize();
	// -------------------------------------------------------------------------	// Makes the table resizable. The load on the table is computed
	// when  are added or removed and the table is resized if required.
	// -------------------------------------------------------------------------
	// void makeResizable();
protected:
	// -------------------------------------------------------------------------
	//	DESCRIPTION: 	Following members store reference to user supplied hash 
	//					comparison and free functions.
	// -------------------------------------------------------------------------
	Sdf_ty_hashFunc				fpHashFunc;
	Sdf_ty_hashKeyCompareFunc	fpCompareFunc;
	Sdf_ty_hashKeyFreeFunc		fpHashKeyFreeFunc;
	Sdf_ty_hashElemFreeFunc		fpHashElemFreeFunc;
	
	// -------------------------------------------------------------------------
	//	DESCRIPTION: 	Other members.
	//	
	//	numberOfBuckets		-	Number of buckets in the hash.
	//	ppHashChains		-	Array that holds the hash lists
	// -------------------------------------------------------------------------
	Sdf_ty_U32bit 				numberOfBuckets;
	Sdf_ty_U32bit*				bucketCounter;
	Sdf_st_commonHashElement	**ppHashChains;

	// Flag that indicates if the hash table is resizable.
	// Sdf_ty_u8bit				resizable;
	//
	// The minimum and maximum load based on which the table is resized 
	// if it is a resizable table. The load value id the average number
	// of elements in a hash chain. It is computed as, 
	// load = (numberOfElements/numberOfBuckets)
	// Sdf_ty_U32bit				minLoad;
	// Sdf_ty_U32bit				maxLoad;

	// Memory Allocators declaration.
	// NOTE: This should be the last declaration in the class.
};

class Sdf_cl_commonThreadSafeHash : public Sdf_cl_commonHash
{
public:
	// -------------------------------------------------------------------------
	// Constructor: Creates the hash table. Since the hash table stores data
	// as void pointers and is not aware of the type of the data, the user has
	// to provide functions using which the class can generate hashes for the
	// data and for comparing the data.
	// 
	// Parameters are :
	// fpHashFunc:
	// 		The hash function to be used. This is a function provided by the 
	// 		user. This function takes in the data as a void pointer and  
	// 		returns a number based on which the hash map assigns the bucket 
	// 		for the element.
	// fpKeyCompareFunc:
	// 		This is a user provided function. This function is used to compare
	// 		elements in the hash table.
	// fpKeyFreeFunc:
	// 		This user provided function is invoked on the key when an element
	// 		is removed from the hash.
	// fpElemFreeFunc:
	// 		This user provided function is invoked on the element when it is to
	// 		be freed.
	// numberOfBuckets:
	// 		The number of buckets in the hash.
	// enableRecursiveFetch
	// 		If recursive fetching is required for the xFetch then this
	// 		boolean has to be passed as true. This may be required in
	// 		scenarios where, while manipulating one element within the bucket
	// 		someother element within the same bucket may be required. Such 
	// 		kind of	scenarios if exepected then this boolean should be passed as
	// 		Sdf_co_true
	// -------------------------------------------------------------------------
	Sdf_cl_commonThreadSafeHash\
		(\
		Sdf_ty_hashFunc 			fpHashFunc,\
		Sdf_ty_hashKeyCompareFunc 	fpKeyCompareFunc, \
		Sdf_ty_hashKeyFreeFunc 		fpKeyFreeFunc, \
		Sdf_ty_hashElemFreeFunc 	fpElemFreeFunc, \
		Sdf_ty_U32bit 				numBuckets = 13, \
		Sdf_ty_bool					enableRecursiveFetch = Sdf_co_false
		);

	// -------------------------------------------------------------------------
	// 	Destructor: Deletes all elements in the hash table.
	// 	
	// 	Parameters are:
	// 	None.
	// -------------------------------------------------------------------------
	~Sdf_cl_commonThreadSafeHash(void);

	// -------------------------------------------------------------------------
	// 	Method to get number of elements in the table at any instant of time.
	// 	
	// 	Parameters are:
	// 	None.
	// -------------------------------------------------------------------------
	Sdf_ty_S32bit size();

	// -------------------------------------------------------------------------
	// This method add an element into the hash. The key is passed to the 
	// hash function to determine the bucket in which the element will be
	// stored. The key passed here is stored in the hash till the element is 
	// removed. The key is deleted by invoking the user supplied key free
	// function when the element is removed from the hash.
	// 
	// Parameters are :
	// pElement:
	// 		Element to be added.
	// pKey:
	// 		Key to be used for locating the bucket.
	// pCorrelationID:
	//		If the value passed is not NULL the correlatio id is generated 
	//		by the hash table and the same is returned in the
	//		pCorrelationID variable that is passed
	// -------------------------------------------------------------------------
	void add\
		(\
		void 			*pElement,\
		void 			*pKey, \
		Sdf_ty_U32bit	*pCorrelationID = Sdf_co_null
		);


	//-------------------------------------------------------------------------
	// Function:	Sdf_cl_commonThreadSafeHash :: add
	//
	// Description: The other flavours of add interface take a key and 
	// generate a session id using the the bucket counter. This method takes a 
	// correlationId and uses that to insert the element into the hash table. 
	// Further this interface will reset the bucket counter based on session 
	// id that has been passed to ensure that the same id does not get 
	// generated again
	//
	// Parameters are:
	// dCorrelationId			- The correlationId using which the data
	// 							has to be inserted into the hash table
	// 
	// pKey						- Any element has two keys. One is the
	// 							correlation id that is maintained within the 
	// 							hash table and the other is a data specific 
	// 							key for which the hashing fn, comparison fn 
	// 							etc are registered by the module using the 
	// 							hash table. The pKey is the module specific key
	// 
	// pData					- The data that has to be stored in the
	// 							hash table
	//-------------------------------------------------------------------------
	void add\
	(\
	 Sdf_ty_U32bit		dCorrelationId,\
	 void*				pKey,\
	 void*				pData\
	);

	// -------------------------------------------------------------------------
	// FUNCTION:    Sdf_cl_commonThreadSafeHash:: addS
	// 
	// DESCRIPTION: This method is used to add elements to the hash table. 
	//				The addition is done to the end of the list where the
	//				add interface prepends the new hash elements.
	//
	// -------------------------------------------------------------------------
	void addS\
		(\
			void 	*pElement,\
			void	*pKey\
		);

    //--------------------------------------------------------------------------
    // This method adds a unique element into the hash table. Operation of
    // this method is simillar to the operation of the 'add()' method, except
    // that the data being added is validate as being unique (ie., no previous
    // instance of the data must exist), before the addition is made to the
    // hash table
    //
    // Returns :
    // Sdf_co_success:
    //      If the adition to the hash table was successful
    // Sdf_co_fail:
    //      If the data could not be added. The reason is indicated by the
    //      pEcode parameter
    // Parameters are :
	// pElement:
	// 		Element to be added.
	// pKey:
	// 		Key to be used for locating the bucket.
    // pEcode:
    //      Return parameter that indicates the reason for failure. Values are:
    //      Sdf_en_errorDuplicateData   - The data could not be added since
    //      the data already exists
    //--------------------------------------------------------------------------
    Sdf_ty_retVal addUnique(
            void                                *pElement,
            void                                *pKey,
            Sdf_ty_error                        *pEcode );


	 //------------------------------------------------------------------------------
    // This method adds a unique element into the hash table. Operation of
    // this method is simillar to the operation of the 'add()' method, except
    // that the data being added is validate as being unique (ie., no previous
    // instance of the data must exist), before the addition is made to the
    // hash table
    //
    // Returns :
    // Sdf_co_success:
    //      If the adition to the hash table was successful
    // Sdf_co_fail:
    //      If the data could not be added. The reason is indicated by the
    //      pEcode parameter
    // Parameters are :
	// pElement:
	// 		Element to be added.
	// pKey:
	// 		Key to be used for locating the bucket.
	//pCorrelationID:
	//		Correlation id returned for this entry
    // pEcode:
    //      Return parameter that indicates the reason for failure. Values are:
    //      Sdf_en_errorDuplicateData   - The data could not be added since
    //      the data already exists
    //------------------------------------------------------------------------------
    Sdf_ty_retVal addUnique(
            void                                *pElement,
            void                                *pKey,
			Sdf_ty_U32bit						*pCorrelationID,\
            Sdf_ty_error                        *pEcode );


	// -------------------------------------------------------------------------
	// This method extracts and removes an element from the table. Returns the
	// element being removed. Returns Sdf_co_null if the key does not exist or
	// the reference count is greater than zero (i.e. another  thread holds a
	// reference to the element. In this case the element and its key remain in
	// the hash table. If the element is removed, it's key is freed using the
	// free function supplied. This method decrements the ref-count of the
	// element.
	// 
	// Parameters are :
	// pKey:
	// 		Key to be used for locating the bucket.
	// -------------------------------------------------------------------------
	void* remove\
		(\
		void *pKey\
		);

		// -------------------------------------------------------------------------
	// This method extracts and removes an element from the table. Returns the
	// element being removed. Returns Sdf_co_null if the key does not exist or
	// the reference count is greater than zero (i.e. another  thread holds a
	// reference to the element. In this case the element and its key remain in
	// the hash table. If the element is removed, it's key is freed using the
	// free function supplied. This method decrements the ref-count of the
	// element.
	// 
	// Parameters are :
	// dCorrelationID:
	// 		Key to be used for locating the bucket and the element.
	// -------------------------------------------------------------------------
	void* remove\
		(\
		Sdf_ty_U32bit	dCorrelationID\
		);


	// -------------------------------------------------------------------------
	// This method is similar to fetch of common hash. In addition to returning
	// the element,this method increments the elements ref-count.
	//
	// Parameters are :
	// pKey:
	// 		Key to be used for locating the bucket.
	// -------------------------------------------------------------------------
	void* fetch\
		(\
		void *pKey\
		);

	// -------------------------------------------------------------------------
	// This method is similar to fetch of common hash. In addition to returning
	// the element,this method increments the elements ref-count.
	//
	// Parameters are :
	// correlationID:
	// 		Key to be used for locating the bucket.
	// -------------------------------------------------------------------------
	void* fetch\
		(\
		Sdf_ty_U32bit correlationID\
		);


	// -------------------------------------------------------------------------
	// A thread that checks out an element must check it back into the hash to 
	// bring down its reference count. Using this method indicates that the 
	// thread is through with its operations on the data element.
	//
	// Parameters are :
	// pKey:
	// 		Key to be used for locating the bucket.
	// -------------------------------------------------------------------------
	void release\
		(\
		void *pKey\
		);

	// -------------------------------------------------------------------------
	//  Overloaded function useing the correlationID
	// Parameters are :
	// correlationID:
	// 		Key to be used for locating the bucket. 
	// -------------------------------------------------------------------------
	void release\
		(\
		Sdf_ty_U32bit correlationID\
		);
	
	// -------------------------------------------------------------------------
	//  Function to iterate through the entire table. User passes pointer to 
	// function of type, Sdf_ty_retVal function(void *pKey, void *pElement).
	// Passed function is invoked for each key and element in the list.
	// If the supplied function returns Sdf_co_success the iteration continues 
	// to the next element. Returning Sdf_co_fail stops the iteration.
	//
	// Parameters are :
	// fpIteratorFunc:
	// 		The iterator function.
	// -------------------------------------------------------------------------
	void forEach\
		(\
		Sdf_ty_hashIteratorFunc fpIteratorFunc\
		);

	// -------------------------------------------------------------------------	
	// This method initializes a hash iterator structure to point to the first
	// element in the hash table.The data element pointed by the iterator is
	// accessible as, pIterator->pCurrentElement->pElement and the 
	// corresponding key is accessible as, pIterator->pCurrentElement->pKey.
	//
	// Parameters are :
	// pIterator:
	// 		Pointer to the hash iterator structure.
	// -------------------------------------------------------------------------
	void initIterator\
		(\
		Sdf_st_commonHashIterator *pIterator\
		);

    void initIterator(
        SDF_INOUT   void                            *pKey,
        SDF_INOUT   Sdf_st_commonHashIterator       *pIterator );

    // Initializes the hash iterator to point to the first bucket in a chain
    // identified by the correlation id. Operation is simillar to the above 
	// member function
	void initIterator(
		SDF_IN		Sdf_ty_U32bit				dCorrelationId, \
		SDF_INOUT	Sdf_st_commonHashIterator	*pIterator);


	// -------------------------------------------------------------------------
	// This method initializes a hash iterator structure to point to the first
	// element in the hash table and locks that chain .The data element pointed
	// by the iterator is accessible as, pIterator->pCurrentElement->pElement
	// and the corresponding key is accessible as, 
	// pIterator->pCurrentElement->pKey.
	// Parameters are :
	// pIterator:
	// 		Pointer to the hash iterator structure.
	// -------------------------------------------------------------------------
	void xInitIterator\
		(\
		Sdf_st_commonHashIterator *pIterator\
		);
    
	// -------------------------------------------------------------------------
	// Function     : Sdf_cl_commonThreadSafeHash::xInitIterator  
	// Description  : Initialises the hash iterator for a particular chain and
	// 				  locks the chain. 
	// -------------------------------------------------------------------------
	void xInitIterator(
        SDF_INOUT   void                            *pKey,
        SDF_INOUT   Sdf_st_commonHashIterator       *pIterator );
	
	// -------------------------------------------------------------------------
	// Function     : Sdf_cl_commonThreadSafeHash::xInitIterator  
	// Description  : Initialises the hash iterator for a particular chain and
	// 				  locks the chain. 
	// -------------------------------------------------------------------------

	void xInitIterator(
        SDF_IN		Sdf_ty_U32bit                   dCorrelationID,
        SDF_INOUT   Sdf_st_commonHashIterator       *pIterator );

	//-----------------------------------------------------------------------
	// FUNCTION:    Sdf_cl_commonThreadSafeHash :: completeXIteration 
	//
	// DESCRIPTION: This method unlocks the chain if we break out of the
	// 				iteration in the middle.
	//-----------------------------------------------------------------------
	
	void completeXIteration\
		(\
		Sdf_st_commonHashIterator *pIterator\
		);

	// -------------------------------------------------------------------------	
	//  Makes the iterator point to the available node in the hash table. 
	// 
	// Parameters are :
	// pIterator:
	// 		Pointer to the hash iterator structure.
	// -------------------------------------------------------------------------
	void next\
		(\
		Sdf_st_commonHashIterator *pIterator\
		);
	
	
	//-------------------------------------------------------------------------
	// FUNCTION:    Sdf_cl_commonThreadSafeHash :: xNext
	// DESCRIPTION: This method makes the xIterator point to the next element 
	// 				in the hash table.It will not release the lock till all the
	// 				 elements in that bucket are traversed
	//-------------------------------------------------------------------------- 				 
	void xNext\
		(\
		Sdf_st_commonHashIterator *pIterator\
		);

    void nextInChain(
        SDF_INOUT   Sdf_st_commonHashIterator       *pIterator );
	// -------------------------------------------------------------------------
	// This function is similar to fetch function.But here,the bucket
	// remain unlocked when the function returns.In this way, the module
	// which is using this function will get exclusive access to the
	// bucket data elements until it is released through another API.
	// Here refCount is not incremented as this is an exclusive fetch.
	// 
	// Parameters are :
	// pKey:
	// 		Key to be used for locating the bucket.
	// -------------------------------------------------------------------------
	void* xFetch\
		(\
		void *pKey\
		);
	
	// -------------------------------------------------------------------------
	//  Overloaded function useing the correlationID
	// Parameters are :
	// correlationID:
	// 		Key to be used for locating the bucket. 
	// -------------------------------------------------------------------------
	void* xFetch\
		(\
		Sdf_ty_U32bit correlationID\
		);

	// -------------------------------------------------------------------------
	// Methid used to do an exclusive fetch and remove the entry from
	// the hash table
	// Parameters are :
	// correlationID:
	// 		Key to be used for locating the bucket. 
	// -------------------------------------------------------------------------
	void *xFetchRemove\
		(\
		Sdf_ty_U32bit correlationID\
		);
	// -------------------------------------------------------------------------
	// This function is similar to release function.This function is
	// invoked only after a xFetch is done on a key.This method must
	// ensure that the lock is released before the function returns.
	// Here refCountis not decremented. 
	//
	// Parameters are :
	// pKey:
	// 		Key to be used for locating the bucket.
	// -------------------------------------------------------------------------
	void xRelease\
		(\
		void *pKey\
		);
	
	// -------------------------------------------------------------------------
	//  Overloaded function useing the correlationID
	// Parameters are :
	// correlationID:
	// 		Key to be used for locating the bucket. 
	// -------------------------------------------------------------------------
	void xRelease\
		(\
		Sdf_ty_U32bit correlationID\
		);

	// Makes the iterator point to the first node,
	// void initIterator(Sdf_st_commonHashIterator *pIterator);
	// Fixes the size of the table. The table will not be resized if
	// the load goes out of the acceptable range.
	// void fixSize();
	// Makes the table resizable. The load on the table is computed
	// when  are added or removed and the table is resized if required.
	// void makeResizable();

private:
	// -------------------------------------------------------------------------
	//	DESCRIPTION: 	Members variables.
	//	
	//	pBucketLock			-	Bucket level mutex locking array.
	//	hashLock			-	Hash table level locking for number of elements.
	// -------------------------------------------------------------------------
	Sdf_ty_mutexT 	*pBucketLock;
	Sdf_ty_mutexT 	hashLock;

	// Memory Allocators declaration.
	// NOTE: This should be the last declaration in the class.

};

#endif
