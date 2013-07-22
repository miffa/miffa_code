
#include "YsServiceJob.h"
#include "YsService.h"
#include "YsSocketop.h"

void YS_ServiceJob::Run(void* jobdata)
{
	//table  pageid
	//int fd
	YsArgs* args = (YsArgs*) jobdata;
	
	char* bufferOut = (char*)malloc(BUFFER_MAX);   //128K
	if(bufferOut == NULL)
		return ;
	memset(bufferOut, 0, BUFFER_MAX);

	int desLen;
	bool iRetStat = true;
	if( (desLen=YS_Service::GetInstance()->ProcessMsg(args->args.c_str() , &bufferOut, BUFFER_MAX)) < 0)
	{
		desLen = strlen( bufferOut );
		iRetStat = false;
	}

	if(Writen(args->fd, bufferOut, desLen) < 0)
	{
		iRetStat = false;
	}
	
	free(bufferOut);
	bufferOut = NULL;
	delete args;

	//短连接，处理完释放链接
	close(args->fd);
}

long YS_ServiceJob::Writen(int fd, const void *vptr, long n)
{
	long nwritten = 0;
	const char *ptr = (const char *)vptr;	/* can't do pointer arithmetic on void* */
	long nleft = n;
	while (nleft > 0) {
		if ( (nwritten = write(fd, ptr, nleft)) <= 0)
		{
			//EWOULDBLOCK 会不会出现在非阻塞socket 的read中 ，与机制有关，linux没有
			//if(nwritten<0 && (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))      
			//EINTR  当write收到信号被打断,可以继续写
			if(nwritten<0 && errno == EINTR)
			{
				nwritten = 0;  //继续写
			}
			// EAGAIN，当socket是非阻塞时,如返回此错误,表示写缓冲队列已满,
			// 在这里做延时后再重试.
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