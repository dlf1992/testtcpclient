/******************************************************************
* Copyright(c) 2020-2028 ZHENGZHOU Tiamaes LTD.
* All right reserved. See COPYRIGHT for detailed Information.
*
* @fileName: factory.h
* @brief: 工厂类头文件
* @author: dinglf
* @date: 2021-03-08
* @history:
*******************************************************************/
#ifndef _FACTORY_H_
#define _FACTORY_H_

#include <map>
#include "tcpclient.h"

typedef struct ServerInfo
{
	char ip[16];
	unsigned short port;
	pFun pcallback;
	pReadPacketFun preadpacket;
	int recoflag;//0不需要 1需要
	int recointerval;//重连间隔 s
	int maxbufsize;//最大缓冲区大小
}__attribute__((packed)) SERVERINFO;

class Factory
{
private:
	TcpClient *ptcpclient;
	char m_ip[16];
	unsigned short m_port;
	pFun m_pcallback;
	pReadPacketFun m_preadpacket;
	int m_recflag;//重连标志 0 不重连 1 重连
	int m_recinterval;//重连间隔 s
	int m_maxbufsize;
	pthread_t tid;	
	SERVERINFO m_svrinfo;
public:
	Factory(const char* ip,unsigned short port,pFun Callback,int recflag,int recinterval,pReadPacketFun ReadPacket,int maxbufsize);
	~Factory();
	static void *work(void *arg);
	void connect();
	int senddata(const char* senddata,int datalen);
	void disconnect();
};
#endif

