#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */
#include	<sys/time.h>	/* timeval{} for select() */
#include	<sys/select.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<time.h>	
#include	<errno.h>	
#include	<sys/types.h>	/* required for some of our prototypes */
#include	<stdio.h>		/* for convenience */
#include	<unistd.h>		/* for convenience */
#include	<stdlib.h>		/* for convenience */
#include	<string.h>		/* for convenience */
#include <fcntl.h>

#define MAXLINE 2048
#define SA struct sockaddr
 
static int	read_cnt;
static char	*read_ptr;
static char	read_buf[MAXLINE];

void err_sys(char* errMsg)
{
		if(errMsg == NULL)
			printf("error errMsg\n");
		printf("errMsg: %s\n",errMsg);
		exit(127);
};

int setnonblocking(int sock)
{
    int opts;
    opts=fcntl(sock,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        return -1;
    }
    opts = opts|O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts)<0)
    {
        perror("fcntl(sock,SETFL,opts)");
        return -1;
    }
}
/*connect_noblock  to server*/
/*-1:connect error, 0 connect ok*/
int connect_noblock(int fd, const struct sockaddr_in* addr, int sec)
{
	if ( fd < 0 )
		return -1;
		
	if(setnonblocking(fd) <0)
		return -1;
	
	int len = sizeof(sockaddr_in);
	int ret =0;
	if( (ret=connect(fd,(struct sockaddr*)addr, len)) < 0)
	{
		if(errno == EINPROGRESS)
		{
			struct timeval tval;
			tval.tv_sec = sec;
			tval.tv_usec = 0;
			
			fd_set rfdset,wfdset;
			FD_ZERO( &wfdset);
			
			FD_SET(fd, &wfdset);
			rfdset = wfdset;
			
			switch(select(fd+1, &rfdset, &wfdset, NULL, &tval))
			{
				case -1:    //select error
					close(fd); 
					return -1;		
				case  0:    //timeout
					errno = ETIMEDOUT;
					close(fd); 
					return -1;
				default:
					if(FD_ISSET(fd, &wfdset) || FD_ISSET(fd, &rfdset))
					{
						int error;
						socklen_t esize = sizeof(error);
						if( getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &esize) < 0 )
						{
							return -1;
						}
						if(error == 0)
						{
							return 0; //connect ok
						}
						else
						{
							errno = error;
							close(fd); 
							return -1;
						}
					}
					else
					{
						close(fd); 
						return -1;
					}
			}//end switch
		}
		else
		{
			close(fd);
			return -1;//connect error
		}
	}
	return 0;
}

int
main(int argc, char **argv)
{
	int					sockfd, n;
	char				recvline[MAXLINE + 1];
	struct sockaddr_in	servaddr;

	if (argc != 3)
		err_sys("usage: exe  <IPaddress> <port>");

	if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		err_sys("socket error");
		
	setnonblocking(sockfd); //设置非阻塞，需要判断errno
		
	int port = atoi(argv[2]);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(port);	/* daytime server */
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		printf("inet_pton error for %s", argv[1]);
	
	int ret;
	ret=connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	if(ret<0 && errno == EINPROGRESS)
	{
		fd_set wset, rset;
		FD_ZERO(&wset);
		FD_SET(sockfd, &wset);
		struct timeval timeout;
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		rset = wset;
		
		switch(select(sockfd+1,&rset,&wset,NULL,&timeout))
		{
			case -1: //select错误
            case  0:  close(sockfd); exit(-1);break; //超时 推出
            default:
		#ifndef _YOUHUA
            if(!FD_ISSET(sockfd,&wset)) //测试sock是否可读，即是否网络上有数据
            {
				printf("connect error\n");
				close(sockfd);
				exit(0);
            }// end if break;
		#else
			//改进的非阻塞socket connet连接判断
			//如果connect成功，fd变为可写，如果失败fd可读可惜
			if(FD_ISSET(sockfd,&wset) ||FD_ISSET(sockfd,&rset)  )
			{
				int error = 0;
                socklen_t len = sizeof(error);              //防止connect三次握手的第三个ACK包被丢掉。
                if(getsockopt(sock_fd,SOL_SOCKET,SO_ERROR,&error,&len) < 0)      
                 {
                     perror("get sock opt fail:");
                 }
                 if(error == 0)
                 {
                     //printf("connect success\n");
                     //sleep(1);
                 }
                 else
                 {
                 	//printf("connect fail\n");
					close(sockfd);
					exit(0);
                 }
           }
		#endif
		}	
	}
	//printf("connect ok\n");

	sleep(3);
	int counter = 0;
	loop:
	
	long urlid;
	scanf("%ld", &urlid);
	//sprintf(recvline,"client msg[%d]  [%d], wait response", sockfd,counter++);
	sprintf(recvline,"%ld|url_dianping_1|", urlid );
	write(sockfd, recvline,strlen(recvline));
	
	//printf("request msg[%d]  send over\n",counter);
	
	/*
	for (;;) {
		fd_set cli_set;
		FD_ZERO(&cli_set);
		FD_SET(sockfd, &cli_set);
		int r = select (sockfd + 1, &cli_set, NULL, NULL, NULL);
		if (r == -1 && errno == EINTR) continue;
        if (r < 0) {
            exit (1);
        }
		
		if (FD_ISSET (sockfd, &cli_set)) {
			if(read(sockfd, recvline, MAXLINE) >0)
				printf("==msg %\n",  recvline);
		}
	*/
	sleep(1);
	//非阻塞模式使用
	while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
		recvline[n] = 0;	
		printf("%s", recvline);	
	}
	//printf("recv response over\n");	
	if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
	{
		printf("  goto loop \n");
		goto loop;
	}
	else
	{
		//close(sockfd);
		//长连接的话，进行重连
		//
	}
	/*if( (n = read(sockfd, recvline, MAXLINE)) > 0) {
		recvline[n] = 0;	
		printf("recv response[%d](%s)\n",counter-1, recvline);	
		
	goto loop;}*/
	close(sockfd);
	//printf("receive over exit  %d \n", n);
	
	if (n < 0)
		err_sys("read error");

	exit(0);
}
