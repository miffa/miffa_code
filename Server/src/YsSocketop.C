#include <algorithm>
#include "YsSocketop.h"
#include "YsService.h"
#include "YsTools.h"
#include "YsServiceJob.h"
#include "YsThreadPoolManager.h"

YS_SocketEpollQueue* YS_SocketEpollQueue::m_pInstance = NULL;

YS_SocketEpollQueue* YS_SocketEpollQueue::GetInstance()
{
	if(m_pInstance == NULL)
		m_pInstance = new YS_SocketEpollQueue();
	return m_pInstance;
}

YS_SocketEpollQueue::YS_SocketEpollQueue()
{
	pthread_mutex_init(&m_clientLock, NULL);
	m_clientList.reserve(RESERVED_ITEMS);
	pthread_mutex_init(&m_epollLock, NULL);
	m_iEpollFd = INVALID_FD;
	m_iListenFd = INVALID_FD;
	m_iSerPort = DEFAULT_PORT;
}

YS_SocketEpollQueue::~YS_SocketEpollQueue()
{
	CloseServer();
}

bool YS_SocketEpollQueue::OpenServer()
{
	///socket	
	if( (m_iListenFd=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		return false;
		
	int iRetStat = 0, opt = 0, flags=1;
	int sendbuflen = 64*1024;
	int recvbuflen = 64*1024; 
	int blockORnot = true;
	int reuseORnot = true;


    iRetStat = setsockopt(m_iListenFd, SOL_SOCKET, SO_REUSEADDR, &reuseORnot, sizeof(int));
    if(iRetStat < 0)    {
        return false;
    }

    iRetStat = setsockopt(m_iListenFd, SOL_SOCKET, SO_RCVBUF, &recvbuflen, sizeof(int));
    if ( iRetStat < 0)    {
        return false;
    }

    iRetStat = setsockopt(m_iListenFd, SOL_SOCKET, SO_SNDBUF, &sendbuflen, sizeof(int));
    if (iRetStat < 0)    {
        return false;
    }

	if( !SetNoneBlock(m_iListenFd) )
		return false;
	
	struct sockaddr_in    serviceAddr;
    serviceAddr.sin_family = PF_INET;
    serviceAddr.sin_port = htons(m_iSerPort);
    serviceAddr.sin_addr.s_addr = htonl(INADDR_ANY);   //0X00000000  �����ֽ���������ֽ�����ͬ

    iRetStat = bind(m_iListenFd, (struct sockaddr *)&serviceAddr, sizeof(struct sockaddr));
    if(iRetStat < 0)
	{
        close(m_iListenFd);
        return false;
    }
    iRetStat = listen(m_iListenFd, EPOLL_ITEMS);
    if(iRetStat < 0)
	{
        close(m_iListenFd);
        return false;
    }
	
	return AddEpollItem(m_iListenFd);
}

bool YS_SocketEpollQueue::CloseServer()
{
	if( m_iListenFd > 0)
	{
		close(m_iListenFd);
		DelEpollItem(m_iListenFd);
		m_iListenFd = INVALID_FD;
	}
	
	//��ѯ�ر����ӿͻ���
	for(int iPos=0; iPos<m_clientList.size(); ++iPos)
	{
		pthread_mutex_lock(&m_clientLock);
		if(m_clientList[iPos].m_bUsed)
		{
			close(m_clientList[iPos].m_iFd);
			m_clientList[iPos].m_bUsed = false;
			DelEpollItem(m_clientList[iPos].m_iFd);
		}
		pthread_mutex_unlock(&m_clientLock);
	}
	return true;
}
	
bool YS_SocketEpollQueue::AddClientItem(int fd)
{
	if(fd < 0)
		return false;
	
	YS_FdClientItem item(fd), itemNoUsed;
	pthread_mutex_lock(&m_clientLock);
	std::vector<YS_FdClientItem>::iterator iPos 
			= find(m_clientList.begin(),m_clientList.end(), item);
	if(iPos != m_clientList.end())
	{
		iPos->m_iFd = fd;
	}
	
	iPos = find(m_clientList.begin(),m_clientList.end(), itemNoUsed);	
	if(iPos != m_clientList.end())
	{
		iPos->m_iFd = fd;
		iPos->m_bUsed = true;
	}
	else
	{
		m_clientList.push_back(item);
	}
	pthread_mutex_unlock(&m_clientLock);
	return true;	
}

bool YS_SocketEpollQueue::DelClientItem(int fd)
{
	if(fd < 0)
		return false;
	
	YS_FdClientItem item(fd);
	pthread_mutex_lock(&m_epollLock);
	std::vector<YS_FdClientItem>::iterator iter 
			= find(m_clientList.begin(),m_clientList.end(), item);
	if(iter != m_clientList.end())
	{
		iter->m_bUsed = false;
		pthread_mutex_unlock(&m_epollLock);
		return true;
	}
	else
	{
		pthread_mutex_lock(&m_epollLock);
		return false;
	}
}

bool YS_SocketEpollQueue::AddEpollItem(int fd)
{
	if(fd < 0)
		return false;
	
	pthread_mutex_lock(&m_epollLock);
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN | EPOLLET;
	if(epoll_ctl(m_iEpollFd, EPOLL_CTL_ADD, fd, &ev) < 0)
	{
		pthread_mutex_unlock(&m_epollLock);
		return false;
	}
	pthread_mutex_unlock(&m_epollLock);
	return true;
}

bool YS_SocketEpollQueue::DelEpollItem(int fd)
{
	if(fd < 0)
		return false;
		
	pthread_mutex_lock(&m_epollLock);
	struct epoll_event ev;
	ev.data.fd = fd;
	//ev.events = EPOLLIN | EPOLLET;
	if(epoll_ctl(m_iEpollFd, EPOLL_CTL_DEL, fd, &ev) < 0)
	{
		pthread_mutex_unlock(&m_epollLock);
		return false;
	}
	pthread_mutex_unlock(&m_epollLock);
	return true;
}
	
bool YS_SocketEpollQueue::Active()
{
	struct epoll_event events[EPOLL_ITEMS];
	int evFds = epoll_wait(m_iEpollFd, events, EPOLL_ITEMS, -1);
	if(evFds < 0)
	{
	
	}
	
	for(int iPos=0; iPos<evFds; ++iPos)
	{
		if( events[iPos].data.fd == m_iListenFd)
		{
			DealConnect(events[iPos].data.fd);
		}
		else
		{
		#ifdef _PTHREAD_OP
			DealRequestWithThread(events[iPos].data.fd);
		#else
			DealRequest(events[iPos].data.fd);
		#endif
		}
	};
	return true;	
}

bool YS_SocketEpollQueue::Init()
{
	m_iEpollFd = epoll_create(EPOLL_ITEMS);

	//�˿����ó�ʼ��
	m_iSerPort = YS_ConfigData::GetInstance()->m_iSerPort;
	
	CloseServer();
	return OpenServer();
}

bool YS_SocketEpollQueue::ReOpenServer()
{
	CloseServer();
	return OpenServer();
}

bool YS_SocketEpollQueue::DealConnect(int fd)
{
	if(fd < 0)
		return false;
	
	sockaddr_in clientAddr;
	int len = sizeof(struct sockaddr_in);
	int clientFd = accept(fd, (struct sockaddr*)&clientAddr, (socklen_t*)&len);
	if(clientFd < 0)
	{
		CloseServer();
		return false;
	}
		
	if( !SetNoneBlock(clientFd) )
	{
		return false;
	}
	
	if( !AddClientItem(clientFd) )
	{
		return false;
	}
	
	if( !AddEpollItem(clientFd) )
	{
		return false;
	}

	return true;
		
}


bool YS_SocketEpollQueue::DealRequest(int fd)
{
	if(fd < 0)
		return false;
	char bufferIn[BUFFER_ZIZE];
	memset(bufferIn, 0, BUFFER_ZIZE);
	int size = Readn(fd, bufferIn, BUFFER_ZIZE-1);
	if(size <0) 
	{
		DelClientItem(fd);
		DelEpollItem(fd);
		close(fd);	
		return false;	
	}
	bufferIn[size] = '\0';
	
	char* bufferOut = (char*)malloc(BUFFER_MAX);   //128K
	if(bufferOut == NULL)
		return false;
	memset(bufferOut, 0, BUFFER_MAX);

	int desLen;
	bool iRetStat = true;
	if( (desLen=YS_Service::GetInstance()->ProcessMsg(bufferIn , &bufferOut, BUFFER_MAX)) < 0)
	{
		desLen = strlen( bufferOut );
		iRetStat = false;
	}

	if(Writen(fd, bufferOut, desLen) < 0)
	{
		iRetStat = false;
	}
	
	free(bufferOut);
	bufferOut = NULL;

	//�����ӣ��������ͷ�����
	DelClientItem(fd);
	DelEpollItem(fd);
	close(fd);	
	return iRetStat;
}

bool YS_SocketEpollQueue::DealRequestWithThread(int fd)
{
	if(fd < 0)
		return false;
	char bufferIn[BUFFER_ZIZE];
	memset(bufferIn, 0, BUFFER_ZIZE);
	int size = Readn(fd, bufferIn, BUFFER_ZIZE-1);
	if(size <0) 
	{
		DelClientItem(fd);
		DelEpollItem(fd);
		close(fd);	
		return false;	
	}
	bufferIn[size] = '\0';
	YS_ServiceJob* newJob = new YS_ServiceJob();
	YsArgs* newArg = new YsArgs();
	newArg->fd = fd;
	newArg->args = bufferIn;
	
	CThreadManage::GetInstance()->Run(newJob, (void*)newArg);//���߳��ͷ�newJob
	return true;
}

bool YS_SocketEpollQueue::SetNoneBlock(int fd)
{
	if(fd < 0)
		return false;
	int ops = fcntl(fd, F_GETFL);
	if(ops < 0)
		return false;
	ops = ops|O_NONBLOCK;
	if(fcntl(fd,F_SETFL, ops) < 0)
		return false;
	return true;
}

/* Write "n" bytes to a descriptor. */
long YS_SocketEpollQueue::Writen(int fd, const void *vptr, long n)
{
	long nwritten = 0;
	const char *ptr = (const char *)vptr;	/* can't do pointer arithmetic on void* */
	long nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0)
		{
			//EWOULDBLOCK �᲻������ڷ�����socket ��read�� ��������йأ�linuxû��
			//if(nwritten<0 && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))      
			//EINTR  ��write�յ��źű����,���Լ���д
			if(nwritten<0 && errno == EINTR)
			{
				nwritten = 0;  //����д
			}
			// EAGAIN����socket�Ƿ�����ʱ,�緵�ش˴���,��ʾд�����������,
			// ����������ʱ��������.
			else if(nwritten<0 && errno == EAGAIN)
			{
				usleep(1000);
				nwritten = 0;
			}
			else
				return(-1);		/* error */
		}

		nleft -= nwritten;
		ptr   += nwritten;
	}
	return(n);
}

/* Read "n" bytes from a descriptor. */
long YS_SocketEpollQueue::Readn(int fd, char *vptr, long n)
{
	long nread = 0;
	char *ptr = vptr;
	long nleft = n;
	long lastRead = 0;
	//ÿ��read()ʱ����Ҫѭ������ֱ����������һ��EAGAIN����Ϊ�˴��¼�������ɣ�
	//��read()���صĶ��������ݳ���С����������ݳ���ʱ��
	//�Ϳ���ȷ����ʱ��������û�������ˣ�Ҳ�Ϳ�����Ϊ���¶��¼��Ѵ������
	while (nleft > 0) {
		if ( (nread = read(fd, ptr, nleft)) == -1)
		{
			if (errno == EINTR)
			{
				nread = 0;		/* interrupt and call read() again */
			}
			else
			{
				//EWOULDBLOCK �᲻������ڷ�����socket ��read�� ��������йأ�linuxû��
				// errno in (EWOULDBLOCK,  EAGAIN), read over\n");
				if(lastRead != 0)
					return lastRead;
				return(-1);
			}
		}
		else if (nread == 0)
		{
			break;				/* EOF */
		}

		lastRead += nread;
		nleft -= nread;
		ptr   += nread;
	}
	return(n - nleft);		/* return >= 0 */
}