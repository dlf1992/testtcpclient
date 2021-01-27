#include "commuclient.h"
#include "tcpclient.h"

TcpClient *ptcpclient = NULL;
static void *work(void *arg);
pFun callback_func;
SERVERINFO m_svrinfo;

int StartTcpClient(const char* ip,unsigned short port,pFun Callback)
{
	callback_func = Callback;
	////printf("callback_func:%p\n",callback_func);
	ptcpclient = new TcpClient;
	if(NULL == ptcpclient)
	{
		//printf("ptcpclient=NULL\n");
		return -1;
	}
	
	memset(&m_svrinfo,0,sizeof(SERVERINFO));
	memcpy(&m_svrinfo.ip,ip,strlen(ip));
	m_svrinfo.port = port;
	//printf("ip=%s,port=%d\n",ip,port);
	pthread_t tid;
	//unsigned short tport = 1122;
	if(pthread_create(&tid, NULL, work, (void *)(&m_svrinfo)) != 0)
	{
		//printf("thread creat error.\n");
		return -1;
	}
	if(pthread_detach(tid) != 0)
	{
		//printf("thread detach error.\n");
		return -1;
	}	
	return 0;
}
int SenddatatoSvr(const char* senddata,int datalen)
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

void *work(void *arg)
{
	prctl(PR_SET_NAME,"TcpClient");
	static int connecttimes = 0;
	if(NULL == ptcpclient)
	{
		return NULL;
	}
	char m_ip[16] = {0};
	unsigned short m_port = 0;
	
	memcpy(m_ip,&(((SERVERINFO*)arg)->ip),strlen(((SERVERINFO*)arg)->ip));
	m_port = ((SERVERINFO*)arg)->port;
	printf("Serverip=%s,Serverport=%d.\n",m_ip,m_port);
	//unsigned short tport = (unsigned short *)arg;
	//printf("tport=%d.\n",tport);
	while(1)
	{
		if(ptcpclient->connectserver(m_ip,m_port,callback_func))
		{
			//printf("client connect success,connecttimes = %d\n",connecttimes);
			ptcpclient->rcvdata();
		}
		else
		{
			//printf("client connect failed,connecttimes = %d\n",connecttimes);
			sleep(10);
		}
		connecttimes++;		
	}
	
	return ptcpclient;
}
