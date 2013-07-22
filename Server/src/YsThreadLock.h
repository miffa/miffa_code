
#ifndef __LOCK_MUTEX_
#define __LOCK_MUTEX_

#include <pthread.h> 

class CThreadMutex
{
	private:
		pthread_mutex_t m_lock;
	public:
		CThreadMutex()
		{
			pthread_mutex_init(&m_lock, NULL);
		}
		~CThreadMutex()
		{
			pthread_mutex_destroy(&m_lock);
		};
		void Lock(){ pthread_mutex_lock(&m_lock);}
		void UnLock(){ pthread_mutex_unlock(&m_lock);}
		pthread_mutex_t& getLockReference(){ return m_lock;};
		pthread_mutex_t* getLockPtr(){ return &m_lock;};
}; 

class CCondition
{
	private:
		pthread_cond_t m_cond;
	public:
		CCondition()
		{
			pthread_cond_init(&m_cond, NULL);
		}
		
		~CCondition()
		{
			pthread_cond_destroy(&m_cond);
		}

		void Wait(CThreadMutex& loock)
		{
			pthread_cond_wait(&m_cond, loock.getLockPtr());
		}
		
		void Signal()
		{
			pthread_cond_signal(&m_cond);
		}
}; 

class CCondition_YML
{
	private:
		pthread_cond_t m_cond;
		pthread_mutex_t m_lock;

	public:
		CCondition_YML()
		{
			pthread_mutex_init(&m_lock, NULL);
			pthread_cond_init(&m_cond, NULL);
		};
		~CCondition_YML()
		{
			pthread_mutex_destroy(&m_lock);
			pthread_cond_destroy(&m_cond);
		};
		
		void Wait()
		{
			pthread_cond_wait(&m_cond, &m_lock);
		}
		
		/*void TimeWait()
		{
			pthread_cond_wait(&m_cond, &m_lock);
		}*/
		
		void Signal()
		{
			pthread_cond_signal(&m_cond);
		}
		void Lock(){ pthread_mutex_lock(&m_lock);}
		void UnLock(){ pthread_mutex_unlock(&m_lock);}
}; 

#endif