/**/
/**/
/**/
/**/
#ifndef _SOCKET_OP_H 
#define _SOCKET_OP_H

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<sys/epoll.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<time.h>	
#include	<errno.h>	
#include	<sys/types.h>
#include	<stdio.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<fcntl.h>
#include	<pthread.h>

#include <vector>
#include <string>
using namespace std;

/*socket 服务器管理类
负责客户端的tcp连接和请求消息处理
*/

class YS_SocketEpollQueue
{
public:

	/**/
	YS_SocketEpollQueue();
	~YS_SocketEpollQueue();
	static YS_SocketEpollQueue* GetInstance();
	
	bool OpenServer();
	bool ReOpenServer();
	bool CloseServer();
	
	bool AddClientItem(int fd);
	bool DelClientItem(int fd);
	
	bool AddEpollItem(int fd);
	bool DelEpollItem(int fd);
	
	bool Active();
	bool Init();
	
	long Readn(int fd, char *vptr, long n);
	long Writen(int fd, const void *vptr, long n);
	bool SetNoneBlock(int fd);
	
private:
	bool DealConnect(int fd);
	bool DealRequest(int fd);
	bool DealRequestWithThread(int fd);
	
private:
	struct YS_FdClientItem
	{
		YS_FdClientItem(int _fd):m_iFd(_fd),m_bUsed(true){}
		YS_FdClientItem():m_iFd(INVALID_FD),m_bUsed(false){}
		int m_iFd;
		bool m_bUsed;
		bool operator ==(const YS_FdClientItem &item) const
		{
			bool b1 = (item.m_bUsed==m_bUsed);
			bool b2 = (item.m_iFd==m_iFd) || (m_bUsed==false);
			return b1 && b2;
		}
	};
	
	
	pthread_mutex_t m_clientLock;  //lock for m_clientList
	std::vector<YS_FdClientItem> m_clientList; //客户端连接fd
	pthread_mutex_t m_lock;
	int m_iListenFd;                         //服务fd
	pthread_mutex_t m_epollLock;
	int m_iEpollFd;                          //epoll fd
	int m_iSerPort;                          //服务监听端口
	
	static const int INVALID_FD = -99;
	static const int RESERVED_ITEMS = 100;
	static const int EPOLL_ITEMS = 100;
	static const int DEFAULT_PORT = 40000;
	
	static YS_SocketEpollQueue* m_pInstance;
};

struct YsArgs
{
	int fd;
	string args;
};

#endif
