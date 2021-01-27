/******************************************************************
* Copyright(c) 2020-2028 ZHENGZHOU Tiamaes LTD.
* All right reserved. See COPYRIGHT for detailed Information.
*
* @fileName: tcpclient.h
* @brief: TCP客户端
* @author: dinglf
* @date: 2020-05-12
* @history:
*******************************************************************/
#ifndef _TCP_CLIENT_H_
#define _TCP_CLIENT_H_
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <netinet/in.h>
//#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/prctl.h>

#include "parsedata.h"
#include "locker.h"
#include "pub.h"
#define MAX_BUFFER 2048  //Buffer的最大字节
class TcpClient
{
private:
	fd_set rest, west;
	int error;
	socklen_t optlen;
	int sock;
	struct sockaddr_in serveraddr;	
	char rcvbuffer[MAX_BUFFER];
	int rcvbuflen;
	ParseData *pParseData;
	pFun m_pCallback;
public:
	bool connectstatus;
	//互斥锁
	mutex_locker variable_locker;
	//接收数据时间戳
	unsigned long Rcvtimestamp;
public:
	TcpClient();
	~TcpClient();
	bool init();
	bool connectserver(const char *ip,unsigned short port,pFun pCallback);
	void rcvdata();
	int senddata(const char* buf,int buflen);
	void disconnect();
};
#endif