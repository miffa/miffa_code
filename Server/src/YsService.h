/**/
/**/
/**/
/**/
#ifndef _SERVICES_H
#define _SERVICES_H

#include <string>
#include "YsThreadLock.h"

using namespace std;

#define CHUNK 16384
#define BUFFER_ZIZE 1024
#define BUFFER_MAX 1024*128  //128k

struct YS_urlData
{
	long m_lUrlId;
	string m_strFilePath;
	long m_lPeekIndex;
}; 

class YS_Service
{
public:
	static YS_Service* GetInstance();
	bool StartUp();
	long ProcessMsg(const char* msg, char** responseMsg, int size);
protected:
	int GetUrlDataFromDB(YS_urlData& data, const char* tableName);
	long GetUrlDataFromFile(YS_urlData& data, char* buffer, int size);
	long GetUrlDataFromGZFile(YS_urlData& data, char** buffer, int size);
	int  InflateRead(char *source, int len, char **dest, int *dest_size, int gzip=1);
	bool DbChecker();

private:
	static YS_Service* m_pInstance; 
	
	//数据库信息
	string m_DbHost;
	string m_dbName;
	string m_userName;
	string m_passwd;
	
	//CThreadMutex m_lock;
};
#endif
