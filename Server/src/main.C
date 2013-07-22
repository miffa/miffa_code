#include "YsFileop.h"
#include "YsService.h"
#include "YsSocketop.h"
#include "YsTools.h"
#include <signal.h>
#include "YsThreadPoolManager.h"
#include "YsThreadJob.h"

int main(int argc, char** argv)
{
	
	printf("thread test begin \n");
	//CThreadManage* manage = new CThreadManage(10); 
	CThreadManage::GetInstance();
	/*sleep(10);
	printf("we will get job  thread  job new \n");
	for(int i=0;i<40;i++) 
	{
		printf("%d thread  job new \n", i);
		CXJob* job = new CXJob(); 
		manage->Run(job,NULL); 
		//sleep(3);
	} 
	printf("thread init ok\n");

	sleep(2); 
	CYJob* job = new CYJob(); 
	manage->Run(job,NULL); 
	manage->TerminateAll(); 

	printf("thread destory ok\n");
	*/
	//Init
	//mysignal(SIGINT, SIG_IGN);
	mysignal(SIGHUP, SIG_IGN);
	YS_ConfigData::GetInstance()->StartUp();
	 YS_ConfigData::GetInstance()->printInfo();
	if(!YS_SocketEpollQueue::GetInstance()->Init())
	{
		printf("init error  \n");
		CThreadManage::GetInstance()->TerminateAll();
		exit(0);
	}
	printf(" YS_SocketEpollQueue::GetInstance()->Init() ok \n");
	if(!YS_Service::GetInstance()->StartUp())
	{
		printf("StartUp error  \n");
		CThreadManage::GetInstance()->TerminateAll();
		exit(0);
	}
	printf("YS_Service::GetInstance()->StartUp()\n");
	//lunxun
	long g_loop = 0;
	for(;;)
	{
		YS_SocketEpollQueue::GetInstance()->Active();
	}
	CThreadManage::GetInstance()->TerminateAll();
	return 0;
}
