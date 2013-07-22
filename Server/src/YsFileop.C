/***************************/
/*                   */
/*                         */
/***************************/
#include "YsFileop.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>

YS_FileProcessor::YS_FileProcessor(const char* filename): m_strFileName(filename), m_filePtr(NULL),m_lFileLen(-1)
{
	
}

YS_FileProcessor::~YS_FileProcessor()
{
	CloseFile();
}
	
bool YS_FileProcessor::OpenForRead()
{
	m_filePtr = fopen(m_strFileName.c_str(), "r");
	if(m_filePtr == NULL)
		return false;
	SetFileLen();
	return true;
}

bool YS_FileProcessor::OpenForWrite()
{
	m_filePtr = fopen(m_strFileName.c_str(), "w");
	if(m_filePtr == NULL)
		return false;
	return true;
}

bool YS_FileProcessor::OpenForAppend()
{
	m_filePtr = fopen(m_strFileName.c_str(), "a");
	if(m_filePtr == NULL)
		return false;
	return true;
}
	
bool YS_FileProcessor::CloseFile()
{
	if(m_filePtr != NULL)
	{
		fclose(m_filePtr);
		m_filePtr = NULL;
	}
}
	
bool YS_FileProcessor::SeekByIndex(long index)
{
	if( fseek(m_filePtr, index, SEEK_SET) < 0 )
		return false;
	return true;
}

int YS_FileProcessor::GetIntFromJavaToC(int data)
{
	long iRetStat = 0;
	char* cPtr = (char*)&iRetStat;
	cPtr[3] = (char) (data & 0xff);//java位置5  对应C++ 位置4
	cPtr[2] = (char) (data >> 8 & 0xff);//java位置6  对应C++ 位置3
	cPtr[1] = (char) (data >> 16 & 0xff);//java位置7  对应C++ 位置2
	cPtr[0] = (char) (data >> 24 & 0xff);//java位置8  对应C++ 位置1
	
	return iRetStat;
}

int YS_FileProcessor::ReadInt()
{
	int iRetValue = 0;
	int iRetStat = fread((void*)&iRetValue, JAVA_INT_BIT, ONE_TIME, m_filePtr);
	if( iRetStat != ONE_TIME)
		return -1;
	return GetIntFromJavaToC(iRetValue);
}

long YS_FileProcessor::GetLongFromJavaToC(long data)
{
	long iRetStat = 0;
	char* cPtr = (char*)&iRetStat;
	cPtr[7] = (char) (data & 0xff);      //java位置1  对应C++ 位置8
	cPtr[6] = (char) (data >> 8 & 0xff); //java位置2  对应C++ 位置7
	cPtr[5] = (char) (data >> 16 & 0xff);//java位置3  对应C++ 位置6
	cPtr[4] = (char) (data >> 24 & 0xff);//java位置4  对应C++ 位置5
	cPtr[3] = (char) (data >> 32 & 0xff);//java位置5  对应C++ 位置4
	cPtr[2] = (char) (data >> 40 & 0xff);//java位置6  对应C++ 位置3
	cPtr[1] = (char) (data >> 48 & 0xff);//java位置7  对应C++ 位置2
	cPtr[0] = (char) (data >> 56 & 0xff);//java位置8  对应C++ 位置1
	
	return iRetStat;
}

long YS_FileProcessor::ReadLong()
{
	long iRetValue;
	int iRetStat = fread((void*)&iRetValue, JAVA_LONG_BIT, ONE_TIME, m_filePtr);
	if( iRetStat != ONE_TIME)
		return -1;
	return GetLongFromJavaToC(iRetValue);
}

bool YS_FileProcessor::ReadByteArray(void* dest, long size)
{
	return ONE_TIME==fread(dest, size, ONE_TIME, m_filePtr);	
}

bool YS_FileProcessor::ReadCharArray(char* dest, long size)
{
	return size==fread(dest, sizeof(char), size, m_filePtr);
}

bool YS_FileProcessor::WriteLine(const char* buff)
{
	fputs(buff, m_filePtr);
	return true;
}

bool YS_FileProcessor::WriteBin(const void* buff, long size)
{
	return ONE_TIME==fwrite(buff, size, ONE_TIME, m_filePtr);
}

long YS_FileProcessor::SetFileLen()
{
	struct stat st;
	if(fstat(fileno(m_filePtr), &st) < 0 )
		return -1;
	m_lFileLen = st.st_size;
	return 0;	
}

bool YS_FileProcessor::IsEnd()
{
	if(feof(m_filePtr))
		return true;
	return false;
}

int YS_FileProcessor::GetFileFd()
{
	return fileno(m_filePtr);
}
