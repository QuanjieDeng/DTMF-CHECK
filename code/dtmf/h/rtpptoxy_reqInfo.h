#ifndef __RTPPROXY_REQINFO_H__
#define __RTPPROXY_REQINFO_H__
#include <string.h>
#include "sdfhashtable.h"
#include "common.h"
#include "clm_log.h"
#include "Dtmf_types.h"


#define Sdf_RTPproxyReqRecordHashBuckets  10000

class PxyReqRecordInfo
{
public:
	unsigned int timestamp_tmp;
	int          end_tag;
	int          i;
	int          end;
	struct sockaddr_in *RTPProxyoutaddr;
	PxyReqRecordInfo()
	{
		timestamp_tmp = 0;
		end_tag=1;
		i = 0;
		end=1;
	}
	static  PxyReqRecordInfo * getInstance()
	{
		if ( NULL == instance )
		{
		    instance = new PxyReqRecordInfo();
		}
		return instance;
	}
	
	~PxyReqRecordInfo();
	void 					releasePxyReqInfo(int  args);
	int               	 	GetPxyReqInfoTableSize();
	int  					handle_msgUDP(char * rcv_udp, int *dtmf_signal,long* timestamp);
	int  					handle_msgUDP(char * rcv_udp, int *dtmf_signal,int* src_port);
	void                	show_RTP_func(RtpHeader_t rtp);
	void           			show_RTP_func_log(RtpHeader_t rtp);
	void  					show_2833_func(RFC2833Payload_t rfc2833);
	void  					show_2833_func_log(RFC2833Payload_t rfc2833);
	
private:
	static  PxyReqRecordInfo * instance;
	
	Sdf_cl_commonThreadSafeHash *m_pPxyReqRecordTable;
};


#endif
