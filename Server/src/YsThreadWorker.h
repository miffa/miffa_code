#ifndef __CWORKERTHREAD__ 
#define __CWORKERTHREAD__ 

#include "YsThreadJob.h"
#include "YsThreadLock.h"
#include "YsThreadPool.h"

enum ThreadState
{
    THREAD_READY = 0, //����
    THREAD_TERMINATED  = 2,// ��ֹ
    THREAD_RUNNING   = 3,//���� 
    THREAD_IDLE  = 4,// ����
    THREAD_SUSPEND  = 4,// ����
}; 

class CThread 
{ 
private: 
    int          m_ErrCode; 
    //Semaphore    m_ThreadSemaphore;  //the inner semaphore, which is used to realize 
    pthread_t    m_ThreadID;     
    bool         m_Detach;       //The thread is detached 
    bool         m_CreateSuspended;  //if suspend after creating 
    char*        m_ThreadName;  
    ThreadState m_ThreadState;      //the state of the thread 
 
protected: 
    void     SetErrcode(int errcode){m_ErrCode = errcode;} 
    static void* ThreadFunction(void*); 
public:  
    CThread(); 
    CThread(bool createsuspended,bool detach); 
    virtual ~CThread(); 
    virtual void Run(void) = 0; 
    void     SetThreadState(ThreadState state){m_ThreadState = state;} 
    bool     Terminate(void);    //Terminate the threa 
    bool     Start(void);        //Start to execute the thread 
    void     Exit(void); 
    bool     Wakeup(void); 
    ThreadState  GetThreadState(void){return m_ThreadState;} 
    int      GetLastError(void){return m_ErrCode;} 
    void     SetThreadName(char* thrname);//{strcpy(m_ThreadName,thrname);} 
    char*    GetThreadName(void){return m_ThreadName;} 
    int      GetThreadID(void){return m_ThreadID;} 
	//�߳����ȼ�
    bool     SetPriority(int priority); 
    int      GetPriority(void); 
	//�̲߳��м���
    int      GetConcurrency(void); 
    void     SetConcurrency(int num); 
 
    bool     Detach(void); 
    bool     Join(void); 
    bool     Yield(void); 
    int      Self(void); 
}; 
 
 
class CWorkerThread:public CThread 
{ 
private: 
    CThreadPool*  m_ThreadPool; 
    CJob*    m_Job; 
    void*    m_JobData;  
     
    //CThreadMutex m_VarMutex;  //if you change m_jod, you should lock m_VarMutex
    bool      m_IsEnd; 
 
protected: 
 
public: 
    CCondition   m_JobCond;   //m_jod ==NULL wait, if set m_job, signal 
    CThreadMutex m_JobMutex; //go running
    CWorkerThread(); 
    virtual ~CWorkerThread(); 
    void Run(); 
    void    SetJob(CJob* job,void* jobdata);  
    CJob*   GetJob(void){return m_Job;} 
    void    SetThreadPool(CThreadPool* thrpool); 
    CThreadPool* GetThreadPool(void){return m_ThreadPool;} 
}; 
 
#endif