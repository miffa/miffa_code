#include "YsThreadWorker.h" 
#include <stdio.h> 
    
CThread::CThread():m_Detach(false),m_CreateSuspended(false)
{
	m_ThreadName = new char[32];
}
CThread::CThread(bool createsuspended,bool detach):m_Detach(detach),m_CreateSuspended(createsuspended)
{
	m_ThreadName = new char[32];
}

CThread::~CThread()
{
	delete [] m_ThreadName;
}

bool CThread::Terminate(void)    //Terminate the threa 
{
	pthread_cancel(m_ThreadID);
}

bool CThread::Start(void)        //Start to execute the thread 
{
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	if (pthread_create(&m_ThreadID,&attr, ThreadFunction,this) == -1)
	{
		perror("Thread: create failed");
	}
	char buff[32];
	buff[0] = '\0';
	sprintf(buff, "thread__%d", m_ThreadID);
	SetThreadName(buff);
}

void CThread::Exit(void)
{
	pthread_exit(NULL); //必须在工作线程中调用， 否则主线程退出
}

void CThread::SetThreadName(char* thrname)
{
	strcpy(m_ThreadName,thrname);
}

bool CThread::Wakeup(void)
{
	
}


bool CThread::SetPriority(int priority)
{
	return true;
}

int CThread::GetPriority(void)
{
	return 0;
}

int CThread::GetConcurrency(void)
{
	return  pthread_getconcurrency();
}

void  CThread::SetConcurrency(int num)
{
	pthread_setconcurrency(num);
}

bool CThread::Detach(void)
{
	pthread_detach(pthread_self());
	return true;
}
bool CThread::Join(void)
{
	printf(" CThread::Join  terminate thr %d\n", m_ThreadID);
	pthread_cancel(m_ThreadID);	
} 
bool CThread::Yield(void)
{
	sched_yield();
} 
int CThread::Self(void)
{
	return m_ThreadID;
} 
void* CThread::ThreadFunction(void* zz)
{
	CThread *p = (CThread *)zz;
	p->Run();
	return NULL;
}

CWorkerThread::CWorkerThread()//:CThread()  
{   
    m_Job = NULL;   
    m_JobData = NULL;   
    m_ThreadPool = NULL;   
    m_IsEnd = false; 
}   
   
CWorkerThread::~CWorkerThread()   
{   
	if(NULL != m_Job)   
		delete m_Job; 
    //if(m_ThreadPool != NULL)   
    //delete m_ThreadPool;   
}   
   
void CWorkerThread::Run()   
{      
    for(;;)   
    {   
        m_JobMutex.Lock();
		while(m_Job == NULL)
		{		
            m_JobCond.Wait(m_JobMutex);
			SetThreadState(THREAD_SUSPEND);
		}
		SetThreadState(THREAD_RUNNING);			
        m_Job->Run(m_JobData);
        m_Job->SetWorkThread(NULL);   
		delete m_Job;  //释放由new构造者生成的job
        m_Job = NULL;
		SetThreadState(THREAD_IDLE);
        m_JobMutex.UnLock();
		
        m_ThreadPool->MoveToIdleList(this);   
   
        /*if(m_ThreadPool->m_IdleList.size() > m_ThreadPool->GetAvailHighNum())   
        {    
            m_ThreadPool->DeleteIdleThread(m_ThreadPool->m_IdleList.size()-m_ThreadPool->GetInitNum());   
        }*/   

    }   
}   
   
void CWorkerThread::SetJob(CJob* job,void* jobdata)   
{   
    m_JobMutex.Lock();   
    m_Job = job;   
    m_JobData = jobdata;   
    job->SetWorkThread(this);
	SetThreadState(THREAD_READY);	
    m_JobMutex.UnLock();   
    m_JobCond.Signal();   
}   
   
void CWorkerThread::SetThreadPool(CThreadPool* thrpool)   
{   
    m_JobMutex.Lock();   
    m_ThreadPool = thrpool;   
    m_JobMutex.UnLock();   
}  

