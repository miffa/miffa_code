#include "YsThreadPoolManager.h"   
#include "YsThreadPool.h"
#include "YsThreadJob.h"
#include <stdio.h>

CThreadManage* CThreadManage::m_pInstance = NULL;
 
CThreadManage::CThreadManage():m_Pool(NULL){   
    m_NumOfThread = 10;   
    m_Pool = new CThreadPool(m_NumOfThread);   
 }   
   
CThreadManage::CThreadManage(int num):m_Pool(NULL){    
    m_NumOfThread = num;
    m_Pool = new CThreadPool(m_NumOfThread);   
}   

CThreadManage* CThreadManage::GetInstance()
{
	if(m_pInstance == NULL)
	{
		static CThreadMutex m_lock;  //静态锁
		m_lock.Lock();
		if(m_pInstance == NULL)
			m_pInstance = new CThreadManage();
		m_lock.UnLock();
	}
	return m_pInstance;
}
  
CThreadManage::~CThreadManage(){   
    m_Pool->TerminateAll();
	if(NULL != m_Pool)
	{	
		delete m_Pool;
		m_Pool = NULL;
	}
}   
   
void CThreadManage::SetParallelNum(int num){   
    m_NumOfThread = num;
}   
   
void CThreadManage::Run(CJob* job,void* jobdata){   
	printf("CThreadManage::Run\n");
    m_Pool->Run(job,jobdata);   
}   
   
void CThreadManage::TerminateAll(void){   
    m_Pool->TerminateAll();   
}  