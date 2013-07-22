#include "YsThreadJob.h"   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


CJob::CJob(void)   
{   
    m_pWorkThread = NULL;   
    m_JobNo = 0;   
    m_JobName = NULL;   
}   
   
CJob::~CJob(){   
    if(NULL != m_JobName)   
    free(m_JobName);   
}   
   
void CJob::SetJobName(char* jobname)   
{   
    if(NULL !=m_JobName)    {   
        free(m_JobName);   
        m_JobName = NULL;   
    }   
   
    if(NULL !=jobname)    {   
        m_JobName = (char*)malloc(strlen(jobname)+1);   
        strcpy(m_JobName,jobname);   
    } 
}  

void CXJob::Run(void* jobdata) 
{
	printf(" ====sleep five=====The Job comes from CXJOB******\n"); 
	sleep(5);
	printf(" ====sleep five=====ok from CXJOB**********\n");
} 

	
void CYJob::Run(void* jobdata) 
{ 
	printf("========== The Job comes from CYJob\n"); 
} 