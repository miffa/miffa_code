#include "YsThreadJob.h"


class YS_ServiceJob:public CJob 
{ 
	public: 
		YS_ServiceJob(){} 
		~YS_ServiceJob(){}
		void Run(void* jobdata);
		long Writen(int fd, const void *vptr, long n);
}; 