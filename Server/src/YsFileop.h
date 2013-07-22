/***************************/
/*类说明                   */
/*文件读写操作类           */
/***************************/
#ifndef _DATABASE_OP_H
#define _DATABASE_OP_H

#include <string>
using namespace std;

class YS_FileProcessor
{
public:
	YS_FileProcessor(const char* filename);
	~YS_FileProcessor();
	
	bool OpenForWrite();
	bool OpenForAppend();
	bool OpenForRead();
	bool CloseFile();
	
	bool SeekByIndex(long index);
	int ReadInt();
	long ReadLong();
	bool ReadByteArray(void* dest, long size);
	bool ReadCharArray(char* dest, long size);
	bool WriteLine(const char* buff);
	bool WriteBin(const void* buff, long size);
	
	//将java long 转换为C long   只适合X86_64
	long GetLongFromJavaToC(long data);
	//将java int 转换为C int
	int GetIntFromJavaToC(int data);
	
	long SetFileLen();
	long GetFileLen(){ return m_lFileLen; };
	bool IsEnd();
	int GetFileFd();
	
private:
	string m_strFileName;
	FILE* m_filePtr;
	long m_lFileLen;
	static const int ONE_TIME = 1;
	static const int JAVA_INT_BIT = 4;
	static const int JAVA_LONG_BIT = 8;
	
};
#endif
