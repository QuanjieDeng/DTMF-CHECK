#include "DatagramSocket.h"
#include "common.h"
#include "Dtmf_types.h"
#include "framework.h"
#include<stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

//DatagramSocket* DatagramSocket::instance = NULL;

//-----------------------------------------
//FUNC:UDPMsgRecv_thread
//DESCRIPTION:  the thread func UDPMsgRecv_thread
//to recv msg  from policy.
//-----------------------------------------
void* UDPMsgRecv_thread(void* pData)
{
	clm_log(CLM_INFO,"Entry func UDPMsgRecv_thread!");
	DatagramSocket		*pclass	= (DatagramSocket *)pData;
	int 			                  recvsize = 0;
	while(1)
	{

		//RtpReq_msg*  rcvmsg = new RtpReq_msg;
		//int len_msg = sizeof(RtpReq_msg);
		char *rcvmsg=new char[UDP_RECV_LEN];
		memset(rcvmsg, 0x00, UDP_RECV_LEN-1);
		long src_port =0;
        app_dtmf_t * tmp_msg = new app_dtmf_t();
		recvsize = pclass->receive((char*)rcvmsg, UDP_RECV_LEN,&src_port);
		if(recvsize > 0)
		{   
    		dtmf_msgFormat* pFormatMsg = new dtmf_msgFormat;
    		pFormatMsg->msgType= DTMF_FromProxyMsg;
            tmp_msg->rcv_msg = rcvmsg;
            tmp_msg->rcv_port = ntohs(src_port);
    		pFormatMsg->pMsgData	= (void*)tmp_msg;
            clm_log(CLM_INFO,"UDPMsgRecv_thread:rcv_port[%d]\n",tmp_msg->rcv_port);
    		framework::getInstance().post(COMPONENT_DTMF, pFormatMsg);
		}
		else
		{	
			printf("---recv error-----%d\n",recvsize);
		}
		
	}

}
int DatagramSocket::setnonblocking(int sockfd)
{
	int opts;
	opts = fcntl(sockfd, F_GETFL);
	if(opts<0)
	{
		perror("fcntl(sockfd,GETFL)");
		exit(1);
	}
	opts = opts|O_NONBLOCK;
	if(fcntl(sockfd,F_SETFL,opts)<0)
	{
		perror("fcntl(sockfd,SETFL,opts)");
		exit(1);
	}

	return 0;
}

//DatagramSocket::DatagramSocket(int port,char *localip,long  rtpproxy_port,char *rtpproxy_ip,bool broadcast, bool reusesock)
DatagramSocket::DatagramSocket(int port,char *localip,bool broadcast, bool reusesock)
{ 
	int retval = -1;
	sockaddr_in addr;
	sock = socket(AF_INET,SOCK_DGRAM,0);
	if ( sock < 0 )
	{
		perror("socket()");
		printf("socket create  error!\n");
	}
	
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr =  inet_addr(localip);
	addr.sin_port = htons(port);
    #if  0

	RTPProxyoutaddr = (sockaddr_in*)malloc( sizeof(sockaddr_in));
	memset(RTPProxyoutaddr, 0, sizeof(sockaddr_in));

	RTPProxyoutaddr->sin_family = AF_INET;
	RTPProxyoutaddr->sin_addr.s_addr = inet_addr(rtpproxy_ip);
	RTPProxyoutaddr->sin_port = htons(rtpproxy_port);
    #endif

	int OptVal = 1;
	if (broadcast)
	{
		retval = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &OptVal, sizeof(OptVal));
	}
	if (reusesock)
	{
		retval = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &OptVal, sizeof(OptVal));
	}

	retval = bind(sock,(struct sockaddr *)&addr,sizeof(addr));
	if ( retval != 0 )
	{
		perror("bind");
		printf("bind error!\n");
	} 
}

//------------------------------------------
//FUNC:~DatagramSocket
//DESCRIPTION: the class of  DatagramSocket  destructor
//func.
//------------------------------------------
DatagramSocket::~DatagramSocket()
{
	close(sock);
}

//------------------------------------------
//FUNC:DatagramSocket::getAddress
//DESCRIPTION:The  func get the local address.
//------------------------------------------
int DatagramSocket::getAddress(const char * name, char * addr)
{
	struct hostent *hp;
	if ((hp = gethostbyname(name)) == NULL) return (0);
	strcpy(addr, inet_ntoa( *( struct in_addr*)( hp->h_addr)));
	return (1);
}

//------------------------------------------
//FUNC:DatagramSocket::getAddress
//DESCRIPTION:The  func get the local address.
//------------------------------------------

const char* DatagramSocket::getAddress(const char * name)
{
	struct hostent *hp;
	if ((hp = gethostbyname(name)) == NULL) return (0);
	strcpy(ip, inet_ntoa( *( struct in_addr*)( hp->h_addr)));
	return ip;
}

//------------------------------------------
//FUNC:DatagramSocket::receive
//DESCRIPTION:The  func  recive message from internate.
//------------------------------------------
long DatagramSocket::receive(char* msg, int msgsize,long * src_port)
{
	int retval  = 0;
	struct sockaddr_in sender;
	socklen_t sendersize = sizeof(sender);
	//printf("receive*****:%d\n",sock);
	retval = recvfrom(sock,msg,msgsize,0, (struct sockaddr *)&sender, &sendersize);
	strcpy(received,inet_ntoa(sender.sin_addr));
    *src_port = sender.sin_port;
	return retval;
}

char* DatagramSocket::received_from()
{
	return received;
}
#if  1
//------------------------------------------
//FUNC:DatagramSocket::send
//DESCRIPTION:The  func  send message to internate.
//------------------------------------------
long DatagramSocket::send(const char* msg, int msgsize,sockaddr_in *addr)
{

	if(NULL  == msg)
	{
		printf("func DatagramSocket::send msg==NULL\n");
		return -1;	
	}
	long  ret = 0;
	try
	{	
	    ret =  sendto(sock, msg, msgsize, 0, (struct sockaddr *)addr, sizeof(*addr));
		if(ret <= 0)
		{
			printf("sendto  func  error!\n");
			perror("sendto:");
		}
		return ret;
	}
	catch(...)
	{
		printf("func DatagramSocket::send  throw error!\n");
	}
}
#endif
//-----------------------------------------
//FUNC:UDPRecvStartThread
//DESCRIPTION: start a  t hread to recv msg from the internate.
//-----------------------------------------
void  DatagramSocket::UDPRecvStartThread(void)
{
	clm_log(CLM_INFO,"Entry func UDPRecvStartThread");
	Sdf_ty_threadT ThreadId;
	Sdf_ty_error   Error;
	pthread_create(&(ThreadId),Sdf_co_null,UDPMsgRecv_thread,(void *)this);
}
//-----------------------------------------
//FUNC:
//DESCRIPTION:
//-----------------------------------------
void DatagramSocket::closeconnect(int fd)
{
	close(fd);
}

//-----------------------------------------
//FUNC:getlocaip
//DESCRIPTION:get the locaolip ,whill beused in initialize the socket.
//-----------------------------------------
int DatagramSocket::getlocaip(char *ip)
{
	int sockfd; 
	if(-1 == (sockfd = socket(PF_INET, SOCK_STREAM, 0)))
	{
		perror( "socket" );
		return -1;
	}
	struct ifreq req;
	struct sockaddr_in *host;
	bzero(&req, sizeof(struct ifreq));
	strcpy(req.ifr_name, "eth0");

	ioctl(sockfd, SIOCGIFADDR, &req);

	host = (struct sockaddr_in*)&req.ifr_addr;

	strcpy(ip, inet_ntoa(host->sin_addr));

	close(sockfd);
	return 1;
}

