/******************************************************************************
** FUNCTION:
** This header file contains the UDP socket  connect  class .and some define.
** 
**
*******************************************************************************
**
** FILENAME:
** 	DatagramSocket.h
**
** DESCRIPTION:
**  This file whill be used when the progran use socket connection with ohter.
**	
**
** DATE 	   	       NAME            
** ----    		 ----              
** 1024/10	  Quanjie.Deng		Creation
**					
**  Copyright (C) 2006 Aricent Inc . All Rights Reserved
*******************************************************************************/


#ifndef DATAGRAMSOCKET_H_INCLUDED
#define DATAGRAMSOCKET_H_INCLUDED

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "clm_log.h"
#include "framework.h"
#include "common.h"


#define TRUE 1
#define FALSE 0

class DatagramSocket
{
private:
    long retval;
    char ip[30];
    char received[30];
    static DatagramSocket *instance;


public:
	int  sock;
	DatagramSocket(int port,char *localip,bool broadcast, bool reusesock);
	~DatagramSocket();

	long receive(char* msg, int msgsize,long * src_port);
	char* received_from();
	long send(const char* msg, int msgsize,sockaddr_in *addr);
	long sendTo(const char* msg, int msgsize, const char* name);
	int getAddress(const char * name, char * addr);
	int getlocaip(char *ip);
	const char* getAddress(const char * name);
	void UDPRecvStartThread(void);
	void  closeconnect(int fd);
    int  setnonblocking(int sockfd);
};

#endif // DATAGRAMSOCKET_H_INCLUDED

