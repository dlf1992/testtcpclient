/*
 * 	commuclient.h
 *
 */
 
/*条件编译*/
#ifndef COMMU_CLIENT_H_
#define COMMU_CLIENT_H_
#include "RingBuffer.h"

#ifdef __cplusplus
extern "C"  //C++
{
#endif
	typedef  int (*pFun)(const char *,int);
	typedef  int (*pReadPacketFun)(TRingBuffer*,char*,int); 
	int StartTcpClient(const char* ip,unsigned short port,pFun Callback,int recflag,int recinterval,pReadPacketFun ReadPacket,int maxbufsize);
	int ConnectSvr(const char* ip,unsigned short port);
	int SenddatatoSvr(const char* ip,unsigned short port,const char* senddata,int datalen);
	void DisConnect(const char* ip,unsigned short port);
	void StopTcpclient(const char* ip,unsigned short port);
#ifdef __cplusplus
}
#endif
 
#endif /* COMMU_CLIENT_H_ */
 