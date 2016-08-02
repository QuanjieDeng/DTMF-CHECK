#ifndef _DB_CONNECT_POOL_
#define _DB_CONNECT_POOL_


#define OTL_ORA10G_R2  //In makefile defined
//#define OTL_ORA10G
//#define OTL_ORA9I // Compile OTL 4.0/OCI9i
//#define OTL_ORA_UTF8 // Enable UTF8 OTL for OCI9i
#define OTL_STREAM_READ_ITERATOR_ON

#include <string>
#include <memory>
#include <sqlite3.h>
#include "threadUtil.h"
#include <pqxx/pqxx>

using namespace pqxx;
using namespace std;

class Connection {
public:	
	int nNumber;
	int nCount;
	connection* hDB;
	
	Connection():nNumber(-1),nCount(0){
		}
	virtual ~Connection(){}	
};

class CdbConncetPool{

public:

protected:

private:
	pthread_mutex_t m_lock;
	
	static auto_ptr<CdbConncetPool> gInstance;
	char    m_DbConString[64];
	SafeQueue<Connection *> m_queue;
	sem_t m_sem;
	bool m_initialized;
	bool m_stop;
	int m_num;
	
public:
	CdbConncetPool(): m_initialized(false),m_stop(false),m_num(10){
		memset(m_DbConString,0x00,64);
		}
	~CdbConncetPool() {
		Stop(); 
		sem_destroy(&m_sem);
		pthread_mutex_destroy(&m_lock);

	}	

	static CdbConncetPool *Instance();

	bool Init();

	bool PutMsg(Connection *pConn);

	bool CreateConnectionPool(char *pDbConString,int nConnNum=1);
	
	Connection *GetConnection();
	
	void ReleaseConnection(Connection *pConn);
	
	void TerminateConnection(Connection *pConn);
	
	Connection *ReCreateConnection(Connection *pConn);
	
	void lock();
	
	void unlock();
	
	bool isInitialized() {
		pthread_mutex_init(&m_lock, NULL);
		return m_initialized;
	}		
	
protected:
	
	
private:
	bool Stop() ;	
	short ConnectDB(connection **hDB);
	short InitDatabase();
	short DisConnectDB(connection *hDB);	
	Connection *CreateConnection();

};


#endif

