/// ----------------------------------------------------------------------------
// 
//  FILENAME:
//  Sdfdb.cpp
// 
//  DESCRIPTION:
// 	 This file contains the achieve of the functions 
//    database sqlite handle
//
//  DATE			NAME				REFERENCE			REASON
//  ----			----				--------			------
// 2014/10/13	
// 
// Copyright:
// ----------------------------------------------------------------------------

#include "Sdfdb.h"
#include "DbConnectPool.h"
#include "clm_log.h"
#include "common.h"
#include <sys/time.h>
#include <unistd.h>
#include <libpq-fe.h>


//---------------------------------------
//FUNC: m_select_callback
//DESCRIPTION: The func whill be callback int
//the func sqlite3_exec(),and  once there have one
//recode whill be invoted.
//---------------------------------------
CDR_DBInfoManager* CDR_DBInfoManager::m_getInstance = NULL;

//----------------------------------------------------------------------------
//                        Hash fuctions for CSCF_RDInfo  Table
//----------------------------------------------------------------------------
//Hash key  function for CSCF_RDInfo  Table

 Sdf_ty_U32bit Sdf_fn_CDR_RDInfo(Sdf_ty_Pvoid a_pKey)
{
	return (Sdf_ty_U32bit)(long) a_pKey;
}
// element free function for CSCF_RDInfo  Table
void Sdf_fn_CSCF_RDInfoElemFree(Sdf_ty_Pvoid pData)
{
	Sdf_st_mmCDR_RDInfo *pCDRRDInfo = (Sdf_st_mmCDR_RDInfo *)pData;
	delete pCDRRDInfo;
}

Sdf_ty_retVal Sdf_fn_DbKeyCompareString (Sdf_ty_Pvoid pKey1, Sdf_ty_Pvoid pKey2)
{
   if (strcmp((char *)pKey1,(char *)pKey2)==0)
       return Sdf_co_success;
   else
       return Sdf_co_fail;

}

Sdf_ty_U32bit Sdf_fn_strHash\
	(\
	void *pInputName\
	)
{
    unsigned long h = 0, g;
	unsigned char* pName = (unsigned char*)pInputName;
	
    while (*pName)
    {
        h = (h << 4) + *pName++;
        if ((g = h & 0xf0000000))
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}


Sdf_ty_U32bit dbHashCalculate(void* a_pKey)
{
	char * tmpString = (char *)a_pKey;
	Sdf_ty_U32bit result = Sdf_fn_strHash(tmpString);
	delete [] (char*)tmpString;
	return result;
}



//--------------------------------------
//The floww part add for the CSCF_ RD ,link with
//Class CDR_DBInfoManager ,include the class all
//the meth's  achieve.
//--------------------------------------
//CDR_DBInfoManager* CDR_DBInfoManager::m_getInstance==NULL;

//--------------------------------------------------
//FUNC:CDR_DBInfoManager
//DESCRIPTION:
//
//--------------------------------------------------
CDR_DBInfoManager::CDR_DBInfoManager()
{	
	//initialize 
	pthread_mutex_init(&m_pCDRInfoTableLock, Sdf_co_null);
	// Initialize  Hashtable
    m_pCDRRecordTable= new Sdf_cl_commonThreadSafeHash\
              (dbHashCalculate,\
              Sdf_fn_DbKeyCompareString,\
              Sdf_co_null, Sdf_fn_CSCF_RDInfoElemFree,\
    		  Sdf_co_CDRRecordHashBuckets,Sdf_co_true);
}

//--------------------------------------------------
//FUNC:~CDR_DBInfoManager
//DRSCRIPTION:
//
//--------------------------------------------------
CDR_DBInfoManager::~CDR_DBInfoManager()
{
	//destory LOCK
	pthread_mutex_destroy(&m_pCDRInfoTableLock);

	//delete the m_pCDRRecordTable
	if (Sdf_co_null != m_pCDRRecordTable)
		delete m_pCDRRecordTable;
}

//--------------------------------------------------
//FUNC:init
//DRSCRIPTION:
//
//--------------------------------------------------

Sdf_ty_retVal CDR_DBInfoManager::init()
{
	return Sdf_co_success;	
}

//--------------------------------------------------
//FUNC:lock
//DRSCRIPTION:
//
//--------------------------------------------------

void CDR_DBInfoManager:: lock()
{
	pthread_mutex_lock(&m_pCDRInfoTableLock);
}

//--------------------------------------------------
//FUNC:unlock
//DRSCRIPTION:
//
//--------------------------------------------------
void CDR_DBInfoManager:: unlock()
{
	pthread_mutex_unlock(&m_pCDRInfoTableLock);
}

//--------------------------------------------------
//FUNC:~CDR_DBInfoManager
//DRSCRIPTION:The func provted
//
//--------------------------------------------------
Sdf_st_mmCDR_RDInfo* CDR_DBInfoManager::CDR_RD_GetRecordInfoByCall_ID(char * pCall_ID)
{
	clm_log(CLM_INFO,"entry CDR_DBInfoManager::CDR_RD_GetRecordInfoByCall_ID\n");
	
	Sdf_st_mmCDR_RDInfo * pCDR_RDInfo = NULL;
	
	pCDR_RDInfo = (Sdf_st_mmCDR_RDInfo *)m_pCDRRecordTable->xFetch (\
												(Sdf_ty_Pvoid)pCall_ID);
	return (pCDR_RDInfo);
}

void CDR_DBInfoManager::releaseCDRRecordTable(char * pDomain)
{
	m_pCDRRecordTable->xRelease((Sdf_ty_Pvoid)pDomain);
	return;
}

//-----------------------------------------------
//FUNC:
//DESCRIPTION:
//-----------------------------------------------
int  CDR_DBInfoManager::GetCDRRecordTableSize()
{
	return (this->m_pCDRRecordTable->size());
}

//-----------------------------------------------
//FUNC:
//DESCRIPTION:
//-----------------------------------------------

int CDR_DBInfoManager::initCDR_PQ_Database(char * connectmsg)
{
	clm_log(CLM_INFO,"entry CDR_DBInfoManager::initCDR_PQ_Database:%s\n",connectmsg);
    if(!CdbConncetPool::Instance()->isInitialized())
    {
		if(!CdbConncetPool::Instance()->Init())
		{
			clm_log(CLM_ERROR,"CdbConncetPool init failed");
			return -1;
		}
		if(!CdbConncetPool::Instance()->CreateConnectionPool(connectmsg,5))
		{
			clm_log(CLM_ERROR,"CreateConnectionPool failed");
			return -1;		
		}
    }

	#if  0
	int  ret = 0;
	ret = this->LoadCSCF_RDConfigData();
	if(ret < 0)
	{
		return -1;
	}
	#endif
}

int CDR_DBInfoManager::LoadCSCF_RDConfigData(void)
{
	clm_log(CLM_INFO, "Entry func LoadCSCF_RDConfigData\n");
	
	char  sql[SQL_line];
	memset(sql,0,sizeof(sql));
	struct timeval start;
	struct timeval stop;
	sprintf(sql,"select * from %s ", cscf_rd_config_table);
	
	Connection *pConn=CdbConncetPool::Instance()->GetConnection();	
	if(NULL==pConn)
	{
		clm_log(CLM_WARNING,"Get DbConnection Failed!");	
		return -1;
	}
	clm_log(CLM_WARNING,"Get DbConnection success[hNum=%d]!",pConn->nNumber);
	try
	{
		//nontransaction* m_com = new nontransaction(*(pConn->hDB));
		work m_com(*(pConn->hDB));
		gettimeofday(&start, NULL);	
		pqxx::result* Res = new  pqxx::result(m_com.exec(sql));
		m_com.commit();
		gettimeofday(&stop, NULL);
		//printf("CSCF-RDsize:%d\n",Res->size());
	
		for (pqxx::result::const_iterator c = Res->begin(); c != Res->end(); ++c)
	  	{
	  		Sdf_st_mmCSCF_RDInfo *  pCSCF_RDInfo = NULL;
    		pCSCF_RDInfo = new Sdf_st_mmCSCF_RDInfo;
	  		for(pqxx::tuple::const_iterator field = c->begin(); field != c->end();  ++field)
	  		{
	  			int  ret = 0;
	  			if((ret =strcmp(field->name(),"titles"))==0)
	  			{
	  				strcpy(pCSCF_RDInfo->pTitle, field->c_str());
	  			}
				else if((ret =strcmp(field->name(),"domain"))==0)
	  			{
	  				strcpy(pCSCF_RDInfo->pDomain, field->c_str());
	  			}
				else if((ret =strcmp(field->name(),"ip"))==0)
	  			{
	  				strcpy(pCSCF_RDInfo->pIp, field->c_str());
	  			}
				else if((ret =strcmp(field->name(),"port"))==0)
	  			{
	  				pCSCF_RDInfo->port=atoi(field->c_str());
	  			}
				else if((ret =strcmp(field->name(),"regpxy_yn"))==0)
	  			{
	  				pCSCF_RDInfo->bRegProxy=atoi(field->c_str());
	  			}
				else if((ret =strcmp(field->name(),"byname"))==0)
	  			{
	  				strcpy(pCSCF_RDInfo->pByname, field->c_str());
	  			}
	  		}
			Sdf_ty_error pEcode;
			m_pCDRRecordTable->addUnique
				((Sdf_ty_Pvoid)pCSCF_RDInfo,(Sdf_ty_Pvoid)pCSCF_RDInfo->pDomain,&pEcode);
        }
		clm_log(CLM_INFO,"sql:%s usedtime:%d",sql,((stop.tv_sec-start.tv_sec)*1000000+(stop.tv_usec-start.tv_usec)));
		delete  Res;
		//delete  m_com;
	}
	catch (const std::exception &e)
   {
   	  cerr << e.what()<< std::endl;
	  clm_log(CLM_ERROR,"func getDataFromPQDatabase throw a error!\n");
	  CdbConncetPool::Instance()->ReleaseConnection(pConn);
	  return  -1;
   }
   CdbConncetPool::Instance()->ReleaseConnection(pConn);
   return 0;
	
}

int  CDR_DBInfoManager::handleReloadConf_RD(char * tblname)
{
	if(NULL == tblname)
	{
		clm_log(CLM_ERROR,"handleReloadConf_RD  tblname is NULL!\n");
		return -1;
	}
	int  ret = 0;
	this->lock();
	if((ret=strcmp(tblname,"CSCF_RD"))== 0)
	{

	    delete m_pCDRRecordTable;
		m_pCDRRecordTable= new Sdf_cl_commonThreadSafeHash\
              (dbHashCalculate,\
              Sdf_fn_DbKeyCompareString,\
              Sdf_co_null, Sdf_fn_CSCF_RDInfoElemFree,\
    		  Sdf_co_CDRRecordHashBuckets,Sdf_co_true);
		//-------------------------------------------
		ret = CDR_DBInfoManager::getInstance()->LoadCSCF_RDConfigData();
		if(ret < 0)
		{
			this->unlock();
			return -1;
		}
		else
		{
			this->unlock();
			return 1;
		}
	}
	else
	{
		this->unlock();
		return -1;	
	}
	this->unlock();
	return 1;
}

//-----------------------------------------------
//FUNC:Add_CDR_Record_Info
//DESCRIPTION:
//-----------------------------------------------
int CDR_DBInfoManager::Add_CDR_RecordDetailInfo(void *insertmsg)
{
	clm_log(CLM_INFO, "Entry func Add_CDR_RecordDetailInfo\n");
        T_FromPolicyMsg    *insert_msg = (T_FromPolicyMsg    *)insertmsg;
	if(NULL == insertmsg)
	{
		return CSCF_RD_co_fail;
	}
	char  sql[SQL_line];
	memset(sql,0,sizeof(sql));
	struct timeval start;
	struct timeval stop;
	
	Connection *pConn=CdbConncetPool::Instance()->GetConnection();	
	if(NULL==pConn)
	{
		clm_log(CLM_WARNING,"Get DbConnection Failed!");	
		return CSCF_RD_co_fail;
	}
	clm_log(CLM_WARNING,"Get DbConnection success[hNum=%d]!",pConn->nNumber);
	try
	{
		work m_com(*(pConn->hDB));
		sprintf(sql,"INSERT INTO %s (identif_id,statecode,time_statecode,call_type,caller_number,caller_device,callee_number,business_type) VALUES ('%s','%s','%s','%s','%s','%s','%s','%d');",
		cdr_record_detail_table,
		insert_msg->Identif_ID,
		insert_msg->StateCode,
		insert_msg->StateCode_time,
		insert_msg->Call_type,
		insert_msg->From,
		insert_msg->Devicename,
		insert_msg->To,
		insert_msg->business_type);
               // sprintf(sql,"INSERT INTO %s (time_statecode) VALUES ('2001-09-29 01:09:56');",cdr_record_detail_table);
	
		//gettimeofday(&start, NULL);	
		m_com.exec(sql);
		m_com.commit();
		//gettimeofday(&stop, NULL);
		clm_log(CLM_INFO,"sql:%s",sql);
		//clm_log(CLM_INFO,"sql:%s usedtime:%d",sql,(stop.tv_sec * 1000 + (stop.tv_usec/1000))-(start.tv_sec * 1000 + (start.tv_usec/1000)));
	}
	catch (const std::exception &e)
	{
		cerr << e.what()<< std::endl;
		clm_log(CLM_ERROR,"func getDataFromPQDatabase throw a error!\n");
		CdbConncetPool::Instance()->ReleaseConnection(pConn);
		return CSCF_RD_co_fail;
	}
	CdbConncetPool::Instance()->ReleaseConnection(pConn);
	return CSCF_RD_co_success;
}

//-----------------------------------------------
//FUNC:Add_CDR_Record_Info
//DESCRIPTION:
//-----------------------------------------------
int CDR_DBInfoManager::Add_CDR_RecordInfo(Sdf_Call_CDRInfo    *insert_msg)
{
	clm_log(CLM_INFO, "Entry func Add_CDR_RecordInfo\n");
	if(NULL == insert_msg)
	{
		return CSCF_RD_co_fail;
	}
	char  sql[SQL_line];
	memset(sql,0,sizeof(sql));
	struct timeval start;
	struct timeval stop;
	
	Connection *pConn=CdbConncetPool::Instance()->GetConnection();	
	if(NULL==pConn)
	{
		clm_log(CLM_WARNING,"Get DbConnection Failed!");	
		return CSCF_RD_co_fail;
	}
	clm_log(CLM_WARNING,"Get DbConnection success[hNum=%d]!",pConn->nNumber);
	//Sdf_Call_CDRInfo *pCallCdrInfo = Sdf_co_null;
	//pCallCdrInfo = this->fetchCallCdrInfo(insert_msg->Identif_ID);
	try
	{
		work m_com(*(pConn->hDB));
		sprintf(sql,"INSERT INTO %s (identif_id,call_start_time,call_build_time,final_response,call_type,\
			caller_number,caller_device,callee_number,call_duration,minute,rate,consume_money,business_type) VALUES ('%s','%s','%s','%s','%s','%s','%s','%s','%0.2f','%0.2f','%0.2f','%0.2f','%d');",
		cdr_record_table,
		insert_msg->Identif_ID,
		insert_msg->Time_Start,
		insert_msg->Time_Build,
		insert_msg->StateCode,
		insert_msg->Call_type,
		insert_msg->From,
		insert_msg->Devicename,
		insert_msg->To,
		insert_msg->call_orgduration,
		insert_msg->minute,
		insert_msg->rate,
		insert_msg->consume_money,
		insert_msg->business_type);
	
		//gettimeofday(&start, NULL);	
		m_com.exec(sql);
		m_com.commit();
		//gettimeofday(&stop, NULL);
		clm_log(CLM_INFO,"sql:%s ",sql);
		//clm_log(CLM_INFO,"sql:%s usedtime:%d",sql,(stop.tv_sec * 1000 + (stop.tv_usec/1000))-(start.tv_sec * 1000 + (start.tv_usec/1000)));
	}
	catch (const std::exception &e)
	{
		cerr << e.what()<< std::endl;
		clm_log(CLM_ERROR,"func getDataFromPQDatabase throw a error!\n");
		CdbConncetPool::Instance()->ReleaseConnection(pConn);
		return CSCF_RD_co_fail;
	}
	CdbConncetPool::Instance()->ReleaseConnection(pConn);
	return CSCF_RD_co_success;
}


//-----------------------------------------------
//FUNC:Get_CSCF_RD_RegInfo
//DESCRIPTION:
//-----------------------------------------------
Sdf_Call_RateInfo* CDR_DBInfoManager::Get_CDR_RateInfo(int rate_typeid)
{
	clm_log(CLM_INFO, "Entry func Get_CDR_RateInfo\n");
	Sdf_Call_RateInfo* pTempInfo = NULL;
	pTempInfo = new Sdf_Call_RateInfo;
	char  sql[SQL_line];
	memset(sql,0,sizeof(sql));
	struct timeval start;
	struct timeval stop;
	sprintf(sql,"SELECT * from %s where rate_type_id = '%d'", cdr_rd_rate_table, rate_typeid);

	
	Connection *pConn=CdbConncetPool::Instance()->GetConnection();	
	if(NULL==pConn)
	{
		clm_log(CLM_WARNING,"Get DbConnection Failed!");	
		return NULL;
	}
	clm_log(CLM_WARNING,"Get DbConnection success[hNum=%d]!",pConn->nNumber);
	try
	{
		#if  0
		nontransaction* m_com = new nontransaction(*(pConn->hDB)); 
		gettimeofday(&start, NULL);	
		pqxx::result* Res = new  pqxx::result(m_com->exec(sql));
		//m_com->commit();
		#endif
		work m_com(*(pConn->hDB));
		pqxx::result* Res = new  pqxx::result(m_com.exec(sql));
		m_com.commit();
		gettimeofday(&stop, NULL);	
		for (pqxx::result::const_iterator c = Res->begin(); c != Res->end(); ++c)
	  	{
			pTempInfo->rate_type_id = c["rate_type_id"].as<int>();
		 	strcpy(pTempInfo->callee_pre, c["callee_pre"].as<string>().data());
			pTempInfo->define_minute = c["define_minute"].as<int>();
			strcpy(pTempInfo->effective_time, c["effective_time"].as<string>().data());
			pTempInfo->addi_charge = c["addi_charge"].as<float>();
			pTempInfo->addi_time = c["addi_time"].as<float>();
			strcpy(pTempInfo->access_number, c["access_number"].as<string>().data());
			
			pTempInfo->time_bucket_1 = c["time_bucket_1"].as<int>();
			pTempInfo->time_bucket_2 = c["time_bucket_2"].as<int>();
			pTempInfo->time_bucket_3 = c["time_bucket_3"].as<int>();
			pTempInfo->time_bucket_4 = c["time_bucket_4"].as<int>();
			pTempInfo->time_bucket_5 = c["time_bucket_5"].as<int>();
			
			pTempInfo->base_time_duration_1 = c["base_time_duration_1"].as<int>();
			pTempInfo->base_time_duration_2 = c["base_time_duration_2"].as<int>();
			pTempInfo->base_time_duration_3 = c["base_time_duration_3"].as<int>();
			pTempInfo->base_time_duration_4 = c["base_time_duration_4"].as<int>();
			pTempInfo->base_time_duration_5 = c["base_time_duration_5"].as<int>();
			
			pTempInfo->time_step_1 = c["time_step_1"].as<int>();
			pTempInfo->time_step_2 = c["time_step_2"].as<int>();
			pTempInfo->time_step_3 = c["time_step_3"].as<int>();
			pTempInfo->time_step_4 = c["time_step_4"].as<int>();
			pTempInfo->time_step_5 = c["time_step_5"].as<int>();

			pTempInfo->rate_1 = c["rate_1"].as<float>();
			pTempInfo->rate_2 = c["rate_2"].as<float>();
			pTempInfo->rate_3 = c["rate_3"].as<float>();
			pTempInfo->rate_4 = c["rate_4"].as<float>();
			pTempInfo->rate_5 = c["rate_5"].as<float>();
			
        	}
		clm_log(CLM_INFO,"sql:%s usedtime:%d",sql,((stop.tv_sec-start.tv_sec)*1000000+(stop.tv_usec-start.tv_usec)));
		delete  Res;
		//delete  m_com;
	}
	catch (const std::exception &e)
	{
		cerr << e.what()<< std::endl;
		clm_log(CLM_ERROR,"func getDataFromPQDatabase throw a error[%s]\n", e.what());
		CdbConncetPool::Instance()->ReleaseConnection(pConn);
	}
	CdbConncetPool::Instance()->ReleaseConnection(pConn);
	return pTempInfo;
}


