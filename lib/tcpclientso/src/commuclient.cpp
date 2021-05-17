#include "commuclient.h"
#include "factory.h"

map<string,Factory*> fac_map; //192.168.8.100:8080
mutex_locker fac_locker;


Factory *findptr(const char* ip,unsigned short port)
{
	Factory * pfac = NULL;
	char tmp[32];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%s:%d",ip,port);
	string svraddr = tmp;
	//printf("svraddr = %s\n",svraddr.c_str());

	fac_locker.mutex_lock();	
	map<string,Factory*>::iterator iter;
	iter = fac_map.find(svraddr);
	if(iter != fac_map.end())
	{
		//printf("svraddr exist\n");
		pfac = iter->second;
	}
	else
	{
		printf("not found svraddr\n");				
	}	
	fac_locker.mutex_unlock();
	return pfac;
}
void insertptr(const char* ip,unsigned short port,Factory *pfactory)
{
	char tmp[32];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%s:%d",ip,port);
	string svraddr = tmp;
	//printf("svraddr = %s\n",svraddr.c_str());
	
	fac_locker.mutex_lock();
	fac_map.insert(make_pair(svraddr,pfactory));
	fac_locker.mutex_unlock();	
}
void clearptr(const char* ip,unsigned short port)
{
	Factory * pfac = NULL;
	char tmp[32];
	memset(tmp,0,sizeof(tmp));
	sprintf(tmp,"%s:%d",ip,port);
	string svraddr = tmp;
	//printf("svraddr = %s\n",svraddr.c_str());

	fac_locker.mutex_lock();
	map<string,Factory*>::iterator iter;
	iter = fac_map.find(svraddr);
	if(iter != fac_map.end())
	{
		//printf("svraddr exist\n");
		pfac = iter->second;
		//printf("pfac = %p\n",pfac);
		fac_map.erase(iter);
		delete pfac;
		pfac = NULL;
		//printf("---------------\n");
	}
	else
	{
		printf("not found svraddr\n");			
	}	
	fac_locker.mutex_unlock();	
}
int StartTcpClient(const char* ip,unsigned short port,pFun Callback,int recflag,int recinterval,pReadPacketFun ReadPacket,int maxbufsize)
{
	Factory *pfactory = NULL;
	pfactory = 	findptr(ip,port);
	if(pfactory == NULL)
	{
		pfactory = new Factory(ip,port,Callback,recflag,recinterval,ReadPacket,maxbufsize);
	}
	if(NULL == pfactory)
	{
		return -1;
	}
	insertptr(ip,port,pfactory);
	return 0;
}
int ConnectSvr(const char* ip,unsigned short port)
{
	Factory *pfactory = NULL;
	pfactory = 	findptr(ip,port);
	if(NULL == pfactory)
	{
		return -1;
	}
	pfactory->connect();
	return 0;
}
int SenddatatoSvr(const char* ip,unsigned short port,const char* senddata,int datalen)
{
	Factory *pfactory = NULL;
	pfactory = 	findptr(ip,port);
	if(NULL == pfactory)
	{
		return -1;
	}
	int ret = pfactory->senddata(senddata,datalen);	
	return ret;
}
void DisConnect(const char* ip,unsigned short port)
{
	Factory *pfactory = NULL;
	pfactory = 	findptr(ip,port);
	if(NULL == pfactory)
	{
		return;
	}
	pfactory->disconnect();
}
void StopTcpclient(const char* ip,unsigned short port)
{
	Factory *pfactory = NULL;
	pfactory = 	findptr(ip,port);
	if(NULL == pfactory)
	{
		return;
	}
	clearptr(ip,port);
}

