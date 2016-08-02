//-----------------------------------------------------------------------------
// File name    : Sdfdb.h                                              
//                                                                            
// Description  : 
//      This file has the db(sqlite)_handle application class declaration.
//                                                                            
//                                                                            
// Date         Name            Reference               Description           
// ---------    -----------     -------------           ----------------      
// 2014/10/13  		                       		Initial Creation      
//                                                                            
//                                                                            
//      Copyright:
//-----------------------------------------------------------------------------
#ifndef __SDFDB_H__
#define __SDFDB_H__

#include "Sdfdbtype.h"
#include "DbConnectPool.h"
#include "threadUtil.h"
#include "sdfhashtable.h"
#include <pqxx/pqxx>
#include <stdlib.h>

using namespace pqxx;
using namespace  std;

#define Sdf_co_CDRRecordHashBuckets  10000


//---------------------------------------
//DESCRIPTON: Class  CDR_DBInfoManager
//DESCRIPTION: This class create to handle the 
//SQLite of  CSCF_RD configinfomation.
//---------------------------------------
//---------------------------------------
//STRUCE: Sdf_st_mmCDR_RDInfo 
//---------------------------------------

class CDR_DBInfoManager
{
	public:
		work* txn;
		connection* PQ_con;
		
		Sdf_cl_commonThreadSafeHash *m_pCDRRecordTable;

		CDR_DBInfoManager();
		~CDR_DBInfoManager();
		 
		static CDR_DBInfoManager *getInstance()
		{
			if ( m_getInstance == NULL )
			{
				m_getInstance = new CDR_DBInfoManager();
			}
			return m_getInstance;
		}
		//-------------------------------------------
		//DESCRIPTION: init the database of the CSCF_RD and 
		//load the data and save in the  ''m_pConfigTable''
		//-------------------------------------------
		virtual Sdf_ty_retVal init();

		//-------------------------------------------
		//DESCRIPTION: lock the table  m_pCDRRecordTable
		//-------------------------------------------
		void lock();
		//-------------------------------------------
		//DESCRIPTION: unlock  the table  m_pCDRConfigTable
		//-------------------------------------------
		void unlock();

		//-------------------------------------------
		//:DESCRIPTION:  The function CDR_RD_GetRecordInfoByCall_ID
		//get the infomation m_pCDRRecordTable of port 
		//by Call_ID.
		//-------------------------------------------
		Sdf_st_mmCDR_RDInfo*   CDR_RD_GetRecordInfoByCall_ID(char * pData);
		void 			 releaseCDRRecordTable(char * pDomain);
		int                     GetCDRRecordTableSize();
		//-------------------------------------------
		int                     handleReloadConf_RD(char *tblname);
		//add for test connect pq.
		int                     initCDR_PQ_Database(char * connectmsg);
		#if  1
		int                     LoadCSCF_RDConfigData(void);
		#endif
		int                      Add_CDR_RecordDetailInfo(void *insertmsg);
		int                      Add_CDR_RecordInfo(Sdf_Call_CDRInfo    *insert_msg);
		Sdf_Call_RateInfo* Get_CDR_RateInfo(int rate_typeid);
	private:
		pthread_mutex_t m_pCDRInfoTableLock;
		static CDR_DBInfoManager* m_getInstance;
};


#endif

