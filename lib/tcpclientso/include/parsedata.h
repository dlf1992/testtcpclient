/******************************************************************
* Copyright(c) 2020-2028 ZHENGZHOU Tiamaes LTD.
* All right reserved. See COPYRIGHT for detailed Information.
*
* @fileName: parsedata.h
* @brief: 域通信客户端数据接收处理
* @author: dinglf
* @date: 2020-05-12
* @history:
*******************************************************************/
#ifndef _PARSE_DATA_
#define _PARSE_DATA_

#include "RingBuffer.h"

typedef  int (*pFun)(const char *,int);
typedef  int (*pReadPacketFun)(TRingBuffer*,char*,int); 

class ParseData
{
private:
	TRingBuffer *pTRingBuffer;
	
public:
	ParseData();
	~ParseData();
	void dataprocess(char *buffer,int buflen,pFun pCallback,pReadPacketFun readpacket,int maxbuf);
	int ReadPacket(char* szPacket, int iPackLen);
};
#endif