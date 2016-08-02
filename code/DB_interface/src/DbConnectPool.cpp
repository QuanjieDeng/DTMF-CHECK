#include "DbConnectPool.h"
#include "clm_log.h"
#include <errno.h>
#include <iostream>

#define MAXCOUNT 10000
//#define MAXCOUNT 10

auto_ptr<CdbConncetPool> CdbConncetPool::gInstance(new CdbConncetPool);

CdbConncetPool *CdbConncetPool::Instance()
{
    if(0== gInstance.get()) {gInstance.reset(new CdbConncetPool);}
    return gInstance.get();
}


short CdbConncetPool::InitDatabase()
{
	//nothing to do.
	return 0;	
}

short CdbConncetPool::ConnectDB(connection **hDB)
{
	try
	{
		// Create database connection
		clm_log(CLM_INFO,"Connect PQ_db string %s\n",this->m_DbConString);
		//hDB = new connection(m_DbConString);
		*hDB = new connection(m_DbConString);
		if ((*hDB)->is_open())
	  	{
		 	clm_log(CLM_INFO,"Opened PQ_database successfully: %s\n",(*hDB)->dbname());
			(*hDB)->disconnect();
		 	return 0;
        } 
	    else
	    {
           printf("Can't open PQ_database\n");
	   	   clm_log(CLM_INFO,"Can't open PQ_database\n");
           return -1;
        }
		
	}
	catch (const std::exception &e)
   	{
	  	clm_log(CLM_INFO,"func ConnectDB throw a error!\n");
      	cerr << e.what()<< std::endl;
	  	return -1;
   	}
	
	return 0;
}


short CdbConncetPool::DisConnectDB(connection *hDB)
{
	try
	{
		//clm_log(CLM_INFO,"hDB logof\n");
		(hDB)->disconnect();
		delete hDB;
	}
	catch (const std::exception &e)
   {
	  clm_log(CLM_INFO,"func DisConnectDB throw a error!\n");
      cerr << e.what()<< std::endl;
	  return -1;
   }
	return 0;

}

bool CdbConncetPool::Init() {
	//autoLock al(m_lock);

	if (m_initialized) 
		return false;

	int err = sem_init(&m_sem, 0, 0);
	if (0 != err) {
		clm_log(CLM_ERROR,"CdbConncetPool::Init Error: failed to do sem_init.");
		return false;
	}
	InitDatabase();
	m_initialized = true;
	return true;
}


bool CdbConncetPool::PutMsg(Connection *pConn)
{
	if (!m_initialized)
	{
		return false;
	}
	//put message into the queue, and signal semaphore
	this->lock();
	m_queue.PutMsg(pConn);
	this->unlock();
	if(m_stop)
		return true;
	int err = sem_post(&m_sem);
	if (0 != err) {
		clm_log(CLM_ERROR,"CdbConncetPool::PutMsg Error: failed to do sem_post.");
		return false;
	}
	return true;
}

bool CdbConncetPool::Stop() {
	if (!m_initialized)
		return false;
	m_stop=true;
	
	Connection *pConn=NULL;

	this->lock();
	for (int i = 0; i < m_num; i++) {
		//disconnect DB handle
		if (m_queue.GetMsg(pConn)) {
			TerminateConnection(pConn);
		}	
		else
		{
			clm_log(CLM_INFO,"stop for i=%d,m_num=%d\n",i,m_num);
			sleep(1);
			i--;
		}
	}
	this->unlock();
	m_initialized = false;
	return true;
}

Connection *CdbConncetPool::CreateConnection()
{
	clm_log(CLM_INFO,"entry func CreateConnection\n");
	//char szDbConnStr[128 * 3 + 1];
	Connection *pConn= new Connection;
	if(NULL==pConn)
	{
		clm_log(CLM_ERROR,"Create Connection object failed!");
		return NULL;
	}
	if(ConnectDB(&(pConn->hDB))<0)
	{
		delete pConn;
		return NULL;
	}
	clm_log(CLM_INFO,"CreateConnection succed!\n");
	return pConn;
}

Connection *CdbConncetPool::GetConnection()
{
	//clm_log(CLM_INFO,"entry  func GetConnection\n");
_RETYR_:
	int err = sem_wait(&m_sem);
	if (0 != err) {
		clm_log(CLM_ERROR,"CdbConncetPool::Run Error: failed to do sem_wait.");
		if (errno == EINTR)
			goto _RETYR_;
		else
			return NULL;
	}

	if(m_stop)
		return NULL;

	Connection *pConn = NULL;
	this->lock();
	if (m_queue.GetMsg(pConn)) {

		if(pConn->nCount++>MAXCOUNT)
		{
			clm_log(CLM_DEBUG,"GetConnection Times longer than the maximum,Recreate connection...");		
			pConn=ReCreateConnection(pConn);
		}
		this->unlock();
		return pConn;
	}
	this->unlock();
	return NULL;
}

void CdbConncetPool::ReleaseConnection(Connection *pConn)
{	
	if(pConn!=NULL)
		PutMsg(pConn);
}

bool CdbConncetPool::CreateConnectionPool(char *pDbConString,int nConnNum) 
{
	clm_log(CLM_INFO,"endtry func CreateConnectionPool->pDbConString:%s nConnNum:%d\n",pDbConString,nConnNum);
	if (!m_initialized)
		return false;
        m_num = 0;
	strcpy(m_DbConString,pDbConString);
	for(int i=0;i<nConnNum;i++)
	{
		//Create DB handle connect pool
		Connection *pConn=CreateConnection();
		if(NULL==pConn)
		{
			return false;
		}
		pConn->nNumber=i;
		//this->lock();
		PutMsg(pConn);
		//this->unlock();
		m_num++;
	}
	return true;
}

void CdbConncetPool::TerminateConnection(Connection *pConn)
{
	if(NULL==pConn)
		return;
	DisConnectDB(pConn->hDB);
	delete pConn;
	pConn=NULL;
}


Connection *CdbConncetPool::ReCreateConnection(Connection *pConn)
{
	if(NULL==pConn)
		return NULL;
	int nNumber=pConn->nNumber;
	TerminateConnection(pConn);	

	while(1)
	{
		pConn=CreateConnection();
		if(NULL==pConn)
		{
			clm_log(CLM_WARNING,"CreateConnection failed,recreate connection....");
			sleep(1);
		}
		else
		{
			pConn->nNumber=nNumber;
			break;
		}		
	}
	return pConn;
}
void CdbConncetPool :: lock()
{
	pthread_mutex_lock(&m_lock);
}

void CdbConncetPool :: unlock()
{
	pthread_mutex_unlock(&m_lock);
}
