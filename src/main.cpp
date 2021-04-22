#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include "commuclient.h"

using namespace std;
#define MAX_DATA 10
unsigned char send_buf[100];

typedef struct Serverinfoext
{
	char ip[16];
	unsigned short port;
	int recoflag;//0不需要 1需要
	int recointerval;//重连间隔 s
	int maxbufsize;//最大缓冲区大小
}__attribute__((packed)) SERVERINFOEXT;

static void signal_action(int sig, siginfo_t* info, void* p)
{
	static int iSig11En = 1;
	struct sigaction act;
	sigset_t* mask = &act.sa_mask;
	int n = 0;
	printf("sig = %d\n",sig);
	if(11 == sig)
	{
		//printf("sig = 11 ........\n");
		pthread_exit((void*)syscall(SYS_gettid));
	 	sigwait(mask, &n);
		if (iSig11En)
		{
			iSig11En = 0;
		}
		else
		{
			//printf("return signal_action........\n");
			iSig11En = 1;
			return;
		}
	}
	else if((SIGUSR1==sig) || (SIGUSR2==sig))
	{
		//LogInfo("program exit");
		exit(0);
	}

	//printf("signal_action,sig = %d........\n",sig);	
}

static void block_bad_singals()
{
  	sigset_t   signal_mask;
	sigemptyset (&signal_mask);
	sigaddset (&signal_mask, SIGPIPE);
	
	if (pthread_sigmask (SIG_BLOCK, &signal_mask, NULL))
	{
    	//printf("block sigpipe error\n");
	}
}

void signal_Init(void)
{
	 struct sigaction act;

	 sigset_t* mask = &act.sa_mask;
	 act.sa_flags=SA_SIGINFO;     /** 设置SA_SIGINFO 表示传递附加信息到触发函数 **/
	 act.sa_sigaction=signal_action;
	 
	 block_bad_singals();
	 
	 // 在进行信号处理的时候屏蔽所有信号
	 sigemptyset(mask);   /** 清空阻塞信号 **/

	 //添加阻塞信号
	 sigaddset(mask, SIGABRT);
	 sigaddset(mask, SIGHUP);
	 sigaddset(mask, SIGQUIT);
	 sigaddset(mask, SIGILL);
	 sigaddset(mask, SIGTRAP);
	 sigaddset(mask, SIGIOT);
	 sigaddset(mask, SIGBUS);
	 sigaddset(mask, SIGFPE);
	 sigaddset(mask, SIGSEGV);
	 sigaddset(mask, SIGUSR1);
	 sigaddset(mask, SIGUSR2);

	 
	//安装信号处理函数
	 sigaction(SIGABRT,&act,NULL);
	 //sigaction(SIGEMT,&act,NULL);
	 sigaction(SIGHUP,&act,NULL);
	 sigaction(SIGQUIT,&act,NULL);
	 sigaction(SIGILL,&act,NULL);
	 sigaction(SIGTRAP,&act,NULL);
	 sigaction(SIGIOT,&act,NULL);
	 sigaction(SIGBUS,&act,NULL);
	 sigaction(SIGFPE,&act,NULL);
	 sigaction(SIGSEGV,&act,NULL);
	 sigaction(SIGUSR1,&act,NULL);
	 sigaction(SIGUSR2,&act,NULL);
	 /*
	  * linux重启或使用kill命令会向所有进程发送SIGTERM信号，所以不需要安装此信号的处理函数
	  */
	 sigaction(SIGINT,&act,NULL);

}
static int dealpacket(const char* data,int datalen)
{
	printf("dealpacket datalen = %d\n",datalen);
	for(int i=0;i<datalen;i++)
	{
		printf("%02x ",*(data+i));
	}
	printf("\n");
	return 0;

}
static int ReadPacket(TRingBuffer* pTRingBuffer,char* szPacket, int iPackLen)
{
	int iRet = 0;

	int iStartPos = 0;
	int iStopPos = 0;
	int packetlen = 0;
	unsigned char ch1;
	unsigned char ch2;
	unsigned char ch3;
	unsigned char packetlenlow;
	unsigned char packetlenhigh;
	
	if(NULL == pTRingBuffer)
		return iRet;

	if (!pTRingBuffer->FindChar(0x23, iStartPos)) //find #
	{
        //0x23都查找不到 肯定是无效数据 清空
		//printf("can not find #,clear ringbuffer.\n");
        pTRingBuffer->Clear();	
		return iRet;		
	}

	if(pTRingBuffer->GetMaxReadSize() <= iStartPos+6)
	{
		//printf("can not find total protocol.\n");
		//丢弃#前面的数据
		pTRingBuffer->ThrowSomeData(iStartPos);
		return iRet;
	}
	
	if((!pTRingBuffer->PeekChar(iStartPos+1,ch1))\
		||(!pTRingBuffer->PeekChar(iStartPos+2,ch2))\
		||(!pTRingBuffer->PeekChar(iStartPos+3,ch3)))
	{
		//printf("can not find char.\n");
		return iRet;
	}
	
	if((ch1==0x23)&&(ch2==0x23)&&(ch3==0x23))
	{
		//丢弃#前面的数据
		pTRingBuffer->ThrowSomeData(iStartPos);
		iStartPos = 0;
		//数据包长度
		pTRingBuffer->PeekChar(iStartPos+4,packetlenlow);
		pTRingBuffer->PeekChar(iStartPos+5,packetlenhigh);
		packetlen = MAKESHORT(packetlenlow,packetlenhigh);
		if(packetlen > 2048)
		{
			//处理异常数据
			pTRingBuffer->Clear();	
			return iRet;
		}		
		iStopPos = iStartPos+packetlen;
		if (iStopPos <= pTRingBuffer->GetMaxReadSize())
		{
			if (iStopPos > iPackLen) pTRingBuffer->ThrowSomeData(iStopPos); //数据超长，丢弃
			else if (pTRingBuffer->ReadBinary((uint8*)szPacket, iStopPos))
			{
				iRet = packetlen;
				//m_readBuffer.Clear();
			}
		}
	}
	else
	{
		//长度够，但是不完全符合格式，清空
		//printf("imcomplete with data format,clear all.\n");
		pTRingBuffer->Clear();	
		return iRet;	
	}
	return iRet;		
}

static void *worker(void *arg)
{
	prctl(PR_SET_NAME,"TcpClient");
	char m_ip[16] = {0};
	unsigned short m_port = 0;
	int m_recoflag = 0;
	int m_recointerval = 0;
	int m_maxbufsize = 0;
	
	//printf("start worker\n");
	memcpy(&m_ip,&(((SERVERINFOEXT*)arg)->ip),strlen(((SERVERINFOEXT*)arg)->ip));
	m_port = ((SERVERINFOEXT*)arg)->port;
	m_recoflag = ((SERVERINFOEXT*)arg)->recoflag;
	m_recointerval = ((SERVERINFOEXT*)arg)->recointerval;
	m_maxbufsize = ((SERVERINFOEXT*)arg)->maxbufsize;
	//printf("Serverip=%s,Serverport=%d.\n",m_ip,m_port);	
	ConnectSvr(m_ip,m_port);
	//printf("TcpClient thread exit.\n");
	return NULL;
}
int main(int argc,char *argv[])
{	
	if(argc != 6)
	{
		printf("please input param ip port recoflag recointerval.\n");
	}
	printf("ip:%s,port:%d,recoflag:%d,recointerval:%d\n",argv[1],atoi(argv[2]),atoi(argv[3]),atoi(argv[4]));
	//信号初始化
	//signal_Init();
	
	SERVERINFOEXT m_svrinfo;
	memset(&m_svrinfo,0,sizeof(SERVERINFOEXT));
	memcpy(&m_svrinfo.ip,argv[1],strlen(argv[1]));
	m_svrinfo.port = atoi(argv[2]);	
	m_svrinfo.recoflag = atoi(argv[3]);
	m_svrinfo.recointerval = atoi(argv[4]);
	m_svrinfo.maxbufsize = atoi(argv[5]);
	
	StartTcpClient(m_svrinfo.ip,m_svrinfo.port,dealpacket,m_svrinfo.recoflag,m_svrinfo.recointerval,ReadPacket,m_svrinfo.maxbufsize);
	pthread_t tid;
	if(pthread_create(&tid, NULL, worker,(void *)(&m_svrinfo)) != 0)
	{
		printf("thread worker creat error.\n");
		return -1;
	}
	pthread_detach(tid);
	sleep(1);
	while(1)
	{
		unsigned short num;
		for(int i=0;i<MAX_DATA;i++)
		{
			memset(send_buf,0,sizeof(send_buf));
			send_buf[0] = 0x23;
			send_buf[1] = 0x23;
			send_buf[2] = 0x23;
			send_buf[3] = 0x23;
			send_buf[4] = 0xc0;
			send_buf[5] = 0x03;
			num = i;
			send_buf[6] = (unsigned char)num&0xff;
			send_buf[7] = (unsigned char)((num>>8)&0xff);
			int ret = SenddatatoSvr(m_svrinfo.ip,m_svrinfo.port,(char*)send_buf,sizeof(send_buf));
			printf("senddata ret = %d,packet %d\n",ret,i);
			sleep(1);
		}
		DisConnect(m_svrinfo.ip,m_svrinfo.port);
		StopTcpclient(m_svrinfo.ip,m_svrinfo.port);
		sleep(10);
		break;
	}	
	return 0;
}
