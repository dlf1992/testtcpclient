/******************************************************************
* Copyright(c) 2020-2028 ZHENGZHOU Tiamaes LTD.
* All right reserved. See COPYRIGHT for detailed Information.
*
* @fileName: factory.cpp
* @brief: 工厂类实现
* @author: dinglf
* @date: 2021-03-08
* @history:
*******************************************************************/
#include "factory.h"

Factory::Factory(const char* ip,unsigned short port,pFun Callback,int recflag,int recinterval,pReadPacketFun ReadPacket,int maxbufsize)
{
	ptcpclient = new TcpClient;
	memcpy(m_ip,ip,sizeof(m_ip));
	m_port = port;
	m_pcallback = Callback;
	m_preadpacket = ReadPacket;
	m_recflag = recflag;
	m_recinterval = recinterval;
	m_maxbufsize = maxbufsize;

	memset(&m_svrinfo,0,sizeof(SERVERINFO));
	memcpy(&m_svrinfo.ip,ip,strlen(ip));
	m_svrinfo.port = port;
	m_svrinfo.pcallback = Callback;
	m_svrinfo.preadpacket = ReadPacket;
	m_svrinfo.recoflag = recflag;
	m_svrinfo.recointerval = recinterval;
	m_svrinfo.maxbufsize = maxbufsize;	
}
Factory::~Factory()
{
	if(NULL == ptcpclient)
	{
		delete ptcpclient;
		ptcpclient = NULL;
	}
}
void* Factory::work(void *arg)
{		
	Factory *pthis = (Factory *)arg;

	static int connecttimes = 0;
	if(NULL == pthis->ptcpclient)
	{
		return NULL;
	}
	//printf("Serverip=%s,Serverport=%d,recflag = %d,recinterval = %d,m_maxbufsize = %d.\n",pthis->m_ip,pthis->m_port,pthis->m_recflag,pthis->m_recinterval,pthis->m_maxbufsize);

	while(1)
	{
		if(pthis->ptcpclient->connectserver(pthis->m_ip,pthis->m_port,pthis->m_pcallback,pthis->m_preadpacket,pthis->m_maxbufsize))
		{
			//printf("client connect success,connecttimes = %d\n",connecttimes);
			pthis->ptcpclient->rcvdata();
		}
		else
		{
			//printf("client connect failed,connecttimes = %d\n",connecttimes);
			if(pthis->m_recflag != 1)
			{
				break;
			}			
			sleep(pthis->m_recinterval);
		}
		if(pthis->m_recflag != 1)
		{
			break;
		}
		connecttimes++;		
	}
	printf("exit while.\n");
	return pthis->ptcpclient;		
}
void Factory::connect()
{
	if(NULL == ptcpclient)
	{
		return;
	}
	if(pthread_create(&tid, NULL, work, (void *)(this)) != 0)
	{
		//printf("thread creat error.\n");
		return;
	}
	if(pthread_detach(tid) != 0)
	{
		//printf("thread detach error.\n");
		return;
	}	
}
int Factory::senddata(const char* senddata,int datalen)
{
	int ret = 0;
	if(NULL == ptcpclient)
	{
		//printf("ptcpclient=NULL\n");
		return -1;
	}	
	ret = ptcpclient->senddata(senddata,datalen);
	return ret;
}
void Factory::disconnect()
{
	if(NULL == ptcpclient)
	{
		//printf("ptcpclient=NULL\n");
		return;
	}
	ptcpclient->disconnect();
}



