/***************************************************************/
/**                                                           **/
/**                                                           **/
/**                                                           **/
/***************************************************************/
#ifndef __TOOLS_H_
#define __TOOLS_H_

#include <stdio.h>
#include <map>
#include <string>
#include <iconv.h>
#include <iostream>
using namespace std;

struct YS_ConfigData
{
	string m_strDbHost;
	string m_strDbName;
	string m_strUserName;
	string m_strPasswd;
	static YS_ConfigData* m_pInstance;
	
	int m_iSerPort;
	YS_ConfigData();
	~YS_ConfigData(){};
	bool StartUp();
	void printInfo();
	static YS_ConfigData* GetInstance();
};

#define OUTLEN 1024*128
class YS_CodeConverter
{
public:
	//
	YS_CodeConverter(const char *from_charset,const char *to_charset);
	//
	~YS_CodeConverter();
	//
	int Convert(char *inbuf,int inlen,char *outbuf,int& outlen);
private:
	iconv_t m_codeConvertItem;
};

typedef void SigFunc(int);
SigFunc* mysignal(int signo, SigFunc* func);
bool InitSignal();
void onSigUsr2(int);
void onSigChld(int);
void onSigPipe(int);

/*
#define _LOGINFO YS_Loger::GetInstance()->logInfo
class YS_Loger 
{
public:
	static YS_Loger* GetInstance();
	
	void logInfo(const char* fmt,...)
	YS_Loger(){}
	~YS_Loger();
	bool Init();
	bool closeLog();
	bool bakLog();
protected:
	char* getTime()
	{
		time_t ti = time(NULL);
		return ctime(&ti);
	}
private:
	static YS_Loger* ;
	FILE* m_filePtr;
	string m_logPath;
};
int timeToStr(long long &_time,char* timeStr);
int strToTime(const char * str, long long &_time);
*/

#endif


