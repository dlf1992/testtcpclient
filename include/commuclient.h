/*
 * 	commuclient.h
 *
 */
 
/*条件编译*/
#ifndef COMMU_CLIENT_H_
#define COMMU_CLIENT_H_
 
#ifdef __cplusplus
extern "C"  //C++
{
#endif
	typedef struct ServerInfo
	{
		char ip[16];
		unsigned short port;
	}__attribute__((packed)) SERVERINFO;    //
	typedef  int (*pFun)(const char *,int);
	int StartTcpClient(const char* ip,unsigned short port,pFun Callback);
	int SenddatatoSvr(const char* senddata,int datalen);
#ifdef __cplusplus
}
#endif
 
#endif /* COMMU_CLIENT_H_ */
 