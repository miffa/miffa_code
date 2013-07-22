#include "YsThreadPool.h"   
#include "YsThreadJob.h"   
#include "YsThreadWorker.h"
#include <assert.h>
#include <algorithm>
   
CThreadPool::CThreadPool()   
{   
    m_MaxNum = 50;   
    m_AvailLow = 5;   
    m_InitNum=m_AvailNum = 10 ;      
    m_AvailHigh = 20;   
   
    m_BusyList.clear();   
    m_IdleList.clear();   
    for(int i=0;i<m_InitNum;i++){   
        CWorkerThread* thr = new CWorkerThread();   
        thr->SetThreadPool(this);   
        AppendToIdleList(thr);   
        thr->Start();   
    }   
}   
   
CThreadPool::CThreadPool(int initnum)   
{   
    assert(initnum>0 && initnum<=30);   
    m_MaxNum   = 30;   
    m_AvailLow = initnum-10>0?initnum-10:3;   
    m_InitNum=m_AvailNum = initnum ;      
    m_AvailHigh = initnum+10;   
    m_BusyList.clear();   
    m_IdleList.clear();   
    for(int i=0;i<m_InitNum;i++){   
        CWorkerThread* thr = new CWorkerThread();   
        AppendToIdleList(thr);   
        thr->SetThreadPool(this);   
        thr->Start();       //begin the thread,the thread wait for job   
    }   
   
}   
   
CThreadPool::~CThreadPool()   
{   
   TerminateAll();   
}   
   
void CThreadPool::TerminateAll()   
{   
    for(int i=0;i < m_ThreadList.size();i++) {   
        CWorkerThread* thr = m_ThreadList[i];   
		printf(" terminate thr %d\n", thr->GetThreadID());
        thr->Join();   
    }   
    return;   
}   
   
CWorkerThread* CThreadPool::GetIdleThread(void)   
{   
	while(m_IdleList.size() ==0 )   
		m_IdleCond.Wait(m_IdleMutex);   
   
    CWorkerThread* thr = NULL;
	if(m_IdleList.size() > 0 )   
    {   	
        thr = (CWorkerThread*)m_IdleList.front();   
        printf("Get Idle thread %d\n",thr->GetThreadID());     
    }   
   
    m_IdleMutex.UnLock();   
    return thr;   
}   
   
//add an idle thread to idle list   
void CThreadPool::AppendToIdleList(CWorkerThread* jobthread)   
{   
    m_IdleMutex.Lock();   
    m_IdleList.push_back(jobthread);    
    m_ThreadList.push_back(jobthread);   
    m_IdleMutex.UnLock(); 
}   
   
//move and idle thread to busy thread    
void CThreadPool::MoveToBusyList(CWorkerThread* idlethread)   
{   
    m_BusyMutex.Lock();   
    m_BusyList.push_back(idlethread);   
	m_VarMutex.Lock();
    m_AvailNum--;  
	m_VarMutex.UnLock();	
    m_BusyMutex.UnLock();   
      
    m_IdleMutex.Lock();   
    vector<CWorkerThread*>::iterator pos;   
    pos = find(m_IdleList.begin(),m_IdleList.end(),idlethread);   
    if(pos !=m_IdleList.end())   
        m_IdleList.erase(pos);   
   
    m_IdleMutex.UnLock();   
}   
   
void CThreadPool::MoveToIdleList(CWorkerThread* busythread)   
{   
    m_IdleMutex.Lock();   
    m_IdleList.push_back(busythread);
    m_IdleMutex.UnLock(); 
	m_VarMutex.Lock();
    m_AvailNum++;   
	m_VarMutex.UnLock();	
    m_BusyMutex.Lock();   
    vector<CWorkerThread*>::iterator pos;   
    pos = find(m_BusyList.begin(),m_BusyList.end(),busythread);   
    if(pos!=m_BusyList.end())   
        m_BusyList.erase(pos);   
   
    m_BusyMutex.UnLock();   
   
    m_IdleCond.Signal();   
    m_MaxNumCond.Signal();   
}   
   
//create num idle thread and put them to idlelist   
void CThreadPool::CreateIdleThread(int num)   
{   
    for(int i=0;i<num;i++){   
        CWorkerThread* thr = new CWorkerThread();
        thr->SetThreadPool(this);   
        AppendToIdleList(thr);   
        m_VarMutex.Lock();   
        m_AvailNum++;   
        m_VarMutex.UnLock();   
        thr->Start();       //begin the thread,the thread wait for job   
    }    
}   
   
void CThreadPool::DeleteIdleThread(int num)   
{   
    printf("Enter into CThreadPool::DeleteIdleThread\n");   
    m_IdleMutex.Lock();   
    printf("Delete Num is %d\n",num);   
    for(int i=0;i<num;i++){   
        CWorkerThread* thr;   
        if(m_IdleList.size() > 0 ){   
           thr = (CWorkerThread*)m_IdleList.front();   
           printf("Get Idle thread %d\n",thr->GetThreadID());   
        }   
   
        vector<CWorkerThread*>::iterator pos;   
        pos = find(m_IdleList.begin(),m_IdleList.end(),thr);   
        if(pos!=m_IdleList.end())   
            m_IdleList.erase(pos);  
		m_VarMutex.Lock();
        m_AvailNum--; 
		m_VarMutex.UnLock();		
        printf("The idle thread available num:%d \n",m_AvailNum);   
        printf("The idlelist              num:%d \n",m_IdleList.size());   
    }   
   
    m_IdleMutex.UnLock();   
}   
   
void CThreadPool::Run(CJob* job,void* jobdata)   
{   
    //if the busy thread num adds to m_MaxNum,so we should wait
	printf("CThreadPool::Run\n");
	m_IdleMutex.Lock();
    /*if(GetBusyNum() == m_MaxNum)   
    //    m_MaxNumCond.Wait(m_IdleMutex);   

    if(m_IdleList.size()<m_AvailLow)   
    {   
        if(GetAllNum()+m_InitNum-m_IdleList.size() < m_MaxNum )   
            CreateIdleThread(m_InitNum-m_IdleList.size());   
        else   
            CreateIdleThread(m_MaxNum-GetAllNum());   
    }*/
	
    CWorkerThread*  idlethr = GetIdleThread();   
    if(idlethr !=NULL)
    {      
        MoveToBusyList(idlethr);
		printf("Job is set to thread %d \n",idlethr->GetThreadID());
		
        //idlethr->SetThreadPool(this);    
        idlethr->SetJob(job,jobdata);	
    }
	return;	
}   
