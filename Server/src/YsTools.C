

#include "YsTools.h"
#include <stdio.h>
#include<signal.h>
#include<sys/wait.h>

YS_ConfigData* YS_ConfigData::m_pInstance = NULL;
YS_ConfigData* YS_ConfigData::GetInstance() 
{
	if(m_pInstance == NULL)
		m_pInstance = new YS_ConfigData();
	return m_pInstance;
} 

YS_ConfigData::YS_ConfigData()
{
	m_strDbHost = "sdb.lbs.server";
	m_strDbName = "lbs";
	m_strUserName = "guest";
	m_strPasswd = "guest";
	m_iSerPort = 40000;
}
	
bool YS_ConfigData::StartUp()
{
	string strEtcPath = string(getenv("HOME")) + "/Server/etc/ser.conf";
	FILE* filePtr = fopen(strEtcPath.c_str(), "r");
	if(filePtr == NULL)
	{
		return false;
	}
	char buffer[128];
	memset(buffer, 0, 128);
	while( fgets(buffer, 128,filePtr)!= NULL )
	{
		string strItem(buffer);
		string strName("");
		string strValue("");
		int iPos = strItem.find( ":" );
		if ( iPos == string::npos )
			continue;
		int iPos_ = strItem.find( "|" );
		if ( iPos_ == string::npos )
			continue;
		else
		{
			strName =  strItem.substr(0,iPos);
			
			strValue =  strItem.substr(iPos+1, iPos_-iPos-1);
		}
		if(strName == "serport")
			m_iSerPort = atoi(strValue.c_str()); 
		if(strName == "dbname")
			m_strDbName = strValue;		
		if(strName == "username")
			m_strUserName = strValue;
		if(strName == "passwd")
			m_strPasswd = strValue;
		if(strName == "dbhost")
			m_strDbHost = strValue;					
	}
	
	fclose(filePtr);
	return true;
}
 
void YS_ConfigData::printInfo() 
{
	printf( "dbHost:  -%s-\n" , m_strDbHost.c_str());
	printf( "dbname:  -%s-\n" , m_strDbName.c_str());
	printf( "usernsme:  -%s-\n" , m_strUserName.c_str());
	printf( "passwd:  -%s-\n" , m_strPasswd.c_str());
	printf( "serport  -%d-\n" , m_iSerPort);
} 

//编码转换
YS_CodeConverter::YS_CodeConverter(const char *from_charset,const char *to_charset)
{
	m_codeConvertItem = iconv_open(to_charset,from_charset);
}

YS_CodeConverter::~YS_CodeConverter()
{
	iconv_close(m_codeConvertItem);
}

int YS_CodeConverter::Convert(char *cInbuf,int iInlen,char *cOutbuf,int& iOutlen)
{
	printf(" in YS_CodeConverter::Convert\n");
	char **cPin = &cInbuf;
	char **cPout = &cOutbuf;
	memset(cOutbuf,0, iOutlen);
	
	return iconv(m_codeConvertItem,cPin,(size_t *)&iInlen,cPout,(size_t *)&iOutlen);  //-1 error
}

SigFunc* mysignal(int signo, SigFunc* func)
{
	struct sigaction act, oact;

	act.sa_handler = func;
	sigfillset(&act.sa_mask);
	act.sa_flags = 0;
	if ((signo == SIGALRM) || (signo == SIGPIPE))
	{
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	}
	else
	{
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}
	if (sigaction(signo, &act, &oact) < 0) return (SIG_ERR);
	return (oact.sa_handler);
}

void onSigUsr2(int)
{
	if (fork() == 0)
	{
		abort();
	}
}

void onSigChld(int)
{
	int status;
	while (waitpid(-1, &status, WNOHANG) > 0);
}

void onSigPipe(int)
{
// 在信号处理函数中不能使用printf等不可重入的库函数
	const char *prefix1 = "ERROR1 ";
	const char *msg = " received SIGPIPE mysignal.\n";

	write(STDOUT_FILENO, prefix1, strlen(prefix1));
	write(STDOUT_FILENO, msg, strlen(msg));
}

bool InitSignal()
{
	mysignal(SIGPIPE, onSigPipe);
	mysignal(SIGCHLD, onSigChld);
	mysignal(SIGUSR2, onSigUsr2);
	mysignal(SIGINT, SIG_IGN);
	
	return true;
}

/*
YS_Loger* YS_Loger::m_pInstance = NULL;

YS_Loger* YS_Loger::GetInstance()
{
	if(m_pInstance == NULL)
		m_pInstance = new YS_Loger();
	return m_pInstance;
}

void YS_Loger::logInfo(const char* fmt,...)
{
	va_list args;
	va_start(args, fmt);
	printed = vfprintf(m_filePtr, fmt, args);
	va_end(args);

	fflush(fp);
}
	
bool YS_Loger::Init()
{
	
	m_logPath = getenv("HOME")+"/Server/log/services.log";
	m_filePtr = fopen(m_logPath.c_str(), "a");
	if(m_filePtr == NULL)
		return false;
}


bool YS_Loger::closeLog()
{
	
}

bool YS_Loger::bakLog()
{

}
	
int timeToStr(long long &_time,char* timeStr)
{
	struct tm *newtime;
	time_t long_time = _time;
    newtime = localtime(&long_time); // Convert to local time.
	sprintf(timeStr,"%04d-%02d-%02d %02d:%02d:%02d",
		newtime->tm_year+1900,newtime->tm_mon+1,newtime->tm_mday,
		newtime->tm_hour,newtime->tm_min,newtime->tm_sec );
	return 0;
};

int strToTime(const char * str, long long &_time)
{
	char cha[32];
	strcpy( cha, str );
	struct tm etime;
	cha[4] = '\0';
	etime.tm_year = atoi(cha)-1900;
	strcpy( cha, str );
	cha[7] = '\0';
	etime.tm_mon=atoi(cha+5)-1;
	strcpy( cha, str );
	cha[10] = '\0';
	etime.tm_mday = atoi(cha+8);
	strcpy( cha, str );
	cha[13] = '\0';
	etime.tm_hour = atoi(cha+11);
	strcpy(cha, str);
	cha[16] = '\0';
	etime.tm_min = atoi(cha+14);
	strcpy(cha, str);
	etime.tm_sec = atoi(cha+17);
	etime.tm_isdst = 0;
	_time = mktime(&etime);
	return 0;
};

*/


