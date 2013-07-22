#ifndef __CTHREADMANAGE__ 
#define __CTHREADMANAGE__ 
 
#include "YsThreadPool.h" 
#include "YsThreadJob.h"  
 
class CThreadManage 
{ 
private: 
    CThreadPool*    m_Pool; 
    int          m_NumOfThread;
	static CThreadManage* m_pInstance;
	//CThreadMutex m_lock;
 
protected: 
public: 
    void     SetParallelNum(int num); 
    CThreadManage(); 
    CThreadManage(int num); 
    virtual ~CThreadManage(); 
  
    void    Run(CJob* job,void* jobdata); 
    void    TerminateAll(void);
	static CThreadManage* GetInstance();
}; 
 
#endif 

