
#include <stdio.h>
#include <string.h>
#include "YsService.h"
#include "YsFileop.h"
#include "zlib.h"
#include "bzlib.h"
#include "mysql.h"
#include "YsTools.h"

YS_Service* YS_Service::m_pInstance = NULL;

YS_Service* YS_Service::GetInstance()
{
	if(m_pInstance == NULL)
	{
		static CThreadMutex m_lock;
		m_lock.Lock();
		if(m_pInstance == NULL)
			m_pInstance = new YS_Service();
		m_lock.UnLock();
	}
	return m_pInstance;
}

bool YS_Service::StartUp()
{
	m_DbHost = YS_ConfigData::GetInstance()->m_strDbHost;
	m_dbName = YS_ConfigData::GetInstance()->m_strDbName;
	m_userName = YS_ConfigData::GetInstance()->m_strUserName;
	m_passwd = YS_ConfigData::GetInstance()->m_strPasswd;
	
	return DbChecker();
}

long YS_Service::ProcessMsg(const char* msg, char** responseMsg, int size)
{
	if(msg == NULL)
		return -1;
		
	//解析消息
	YS_urlData reData;
	string item(msg);
	string table("url_dianping_1");
	
	int posF = item.find( "|" );
	if ( posF == string::npos )
	{
		sprintf(*responseMsg, "ERROR:request msg error eg: urlID|tablename|", msg);
		return -1;
	}
	int posL = item.find( "|", posF+1 );
	if ( posL == string::npos )
	{
		sprintf(*responseMsg, "ERROR:request msg error eg: urlID|tablename|", msg);
		return -1;
	}
	else
	{
		reData.m_lUrlId = atol(item.substr(0,posF).c_str());	
		table =  item.substr(posF+1, posL-posF-1);
	}

	//数据库数据查询
	int iRetStat = 0;
	if( (iRetStat=GetUrlDataFromDB(reData, table.c_str())) < 0)
	{
		switch (iRetStat)
		{
			case -1:
				sprintf(*responseMsg, "ERROR:connect dabase error (m_lUrlId:%ld)", reData.m_lUrlId);
				break;
			case -2:
				sprintf(*responseMsg, "ERROR:query  error(m_lUrlId:%ld)", reData.m_lUrlId);
				break;
			case -3:
				sprintf(*responseMsg, "ERROR:store result  error(m_lUrlId:%ld)", reData.m_lUrlId);
				break;
			case -4:
				sprintf(*responseMsg, "ERROR:more than one record in db (m_lUrlId:%ld)", reData.m_lUrlId);
				break;
		}
		return -1;
	}
	//文件查询
	//return GetUrlDataFromFile(reData, *responseMsg, size);
	return GetUrlDataFromGZFile(reData, responseMsg, size);
}

int YS_Service::GetUrlDataFromDB(YS_urlData& data, const char* tableName)
{
	//数据库连接
	MYSQL   mysql;      
	mysql_init(&mysql);       
	if(mysql_real_connect(&mysql,m_DbHost.c_str(),m_userName.c_str(),m_passwd.c_str(),m_dbName.c_str(),0,NULL,0))     
	{     
		char cBuffer[BUFFER_ZIZE];
		sprintf(cBuffer, "select id, filepath,beginindex from %s where id=%d",tableName, data.m_lUrlId);
		int iRetStat = mysql_query(&mysql, cBuffer);     
		if(!iRetStat)     
		{     
			mysql_query(&mysql, "set names utf8"); 
			MYSQL_RES *mysqlRet = mysql_store_result(&mysql);     
			if(mysqlRet != NULL)     
			{     
				long lNumRows = mysql_num_rows(mysqlRet);     
				if(lNumRows == 1)     
				{          
					MYSQL_ROW mysql_row = mysql_fetch_row(mysqlRet);
					//data.m_lUrlId == atol(mysql_row[0]);
					data.m_strFilePath = mysql_row[1];
					data.m_lPeekIndex = atol(mysql_row[2]);
				}     
				else
				{     
					mysql_free_result(mysqlRet);
					mysql_close(&mysql);
					return(-4);     
				}     
				mysql_free_result(mysqlRet); 
				mysql_close(&mysql);
				return(0);     
			}     
			else     
			{     
				mysql_close(&mysql);
				return(-3);     
			}     
		}     
		else     
		{     
			mysql_close(&mysql);
			return(-2);     
		}     
	}     
	else     
	{        
		return(-1);     
	} 
	mysql_close(&mysql);
	return 0;
}

long YS_Service::GetUrlDataFromFile(YS_urlData& data, char* buffer, int size)
{
	YS_FileProcessor filePtr(data.m_strFilePath.c_str());
	if( !filePtr.OpenForRead() )
	{
		sprintf(buffer, "ERROR:open %s error", data.m_strFilePath.c_str());
		return -1;
	}
	
	if( !filePtr.SeekByIndex(data.m_lPeekIndex) )
	{
		sprintf(buffer, "ERROR:fseek %s:%lderror", data.m_strFilePath.c_str(), data.m_lPeekIndex);
		return -1;
	}
	
	long length = filePtr.ReadLong();//lenth
	if(length > filePtr.GetFileLen()-data.m_lPeekIndex)
	{
		sprintf(buffer, "ERROR::url len(%ld) larger than left length%ld(%s) ", length, filePtr.GetFileLen()-data.m_lPeekIndex,data.m_strFilePath.c_str());
		return -1;
	}
	
	filePtr.ReadLong(); //url ID
	filePtr.ReadInt(); // url site id
	filePtr.ReadInt(); //flag 
	void* tempbuffer = malloc(length);
	if(tempbuffer == NULL)
	{
		return -1;
	}
	if( !filePtr.ReadByteArray( tempbuffer,length) )
	{
		filePtr.CloseFile();
		free(tempbuffer);
		sprintf(buffer, "ERROR:read file error(%s) ", data.m_strFilePath.c_str());
		return -1;
	}
	filePtr.CloseFile();
	
	//写入文件，执行gzip -d 命令
	char sbuffer[1024];
	sprintf(sbuffer,"%d", getpid());
	
	string fileunzip = string(getenv("HOME")) + string("/Server/temp/urlinfo.") + sbuffer;
	string filezip = fileunzip + ".gz";
	
	//将二进制字节流写入文件
	YS_FileProcessor filegzip(filezip.c_str());
	filegzip.OpenForWrite();
	filegzip.WriteBin(tempbuffer,length);
	filegzip.CloseFile();
	free(tempbuffer);
	
	//将压缩文件解压
	sprintf(sbuffer, "gzip -d %s",filezip.c_str());
	if(system(sbuffer)<0)
	{
		sprintf(buffer, "ERROR:exe %s error", sbuffer);
		return -1;
	}
	
	//读取解压后的文件
	YS_FileProcessor filegunzip(fileunzip.c_str());
	filegunzip.OpenForRead();
	filegunzip.ReadCharArray(buffer,filegunzip.GetFileLen());
	buffer[filegunzip.GetFileLen()] = '\0';
	filegunzip.CloseFile();	
	unlink(fileunzip.c_str());
	
	char buf[BUFFER_MAX];
	memset(buf, 0, BUFFER_MAX);
	int lenU2G = BUFFER_MAX;
	YS_CodeConverter utf8ToGbk = YS_CodeConverter("utf-8","gbk");
	utf8ToGbk.Convert(buffer, filegunzip.GetFileLen(),buf, lenU2G);
	printf("\n zhuang huan ceshi\n");
	printf("\n %s \n",buf);
	
	return filegunzip.GetFileLen();
}

long YS_Service::GetUrlDataFromGZFile(YS_urlData& data, char** buffer, int size)
{
	YS_FileProcessor filePtr(data.m_strFilePath.c_str());
	if( !filePtr.OpenForRead() )
	{
		sprintf(*buffer, "ERROR:open %s error", data.m_strFilePath.c_str());
		return -1;
	}
	
	if( !filePtr.SeekByIndex(data.m_lPeekIndex) )
	{
		sprintf(*buffer, "ERROR:fseek %s:%lderror", data.m_strFilePath.c_str(), data.m_lPeekIndex);
		return -1;
	}
	
	long length = filePtr.ReadLong();//lenth
	if(length > filePtr.GetFileLen()-data.m_lPeekIndex)
	{
		sprintf(*buffer, "ERROR:error:url len(%ld) larger than left length%ld(%s) ", length, filePtr.GetFileLen()-data.m_lPeekIndex,data.m_strFilePath.c_str());
		return -1;
	}
	
	filePtr.ReadLong(); //url ID
	filePtr.ReadInt(); // url site id
	filePtr.ReadInt(); //flag 
	void* vGzipBuffer = malloc(length);
	if(vGzipBuffer == NULL)
	{
		return -1;
	}
	if( !filePtr.ReadByteArray( vGzipBuffer,length) )
	{
		filePtr.CloseFile();
		free(vGzipBuffer);
		sprintf(*buffer, "ERROR:read file error(%s) ", data.m_strFilePath.c_str());
		return -1;
	}
	filePtr.CloseFile();
	
	int iUnzLen = (int)size;

	if(Z_OK != ( InflateRead((char*)vGzipBuffer, length, buffer, &iUnzLen)))
	{
		free(vGzipBuffer);
		return -1;
	}
	free(vGzipBuffer);

#ifdef _GBK
	char cGbkBuf[BUFFER_MAX];
	memset(cGbkBuf, 0, BUFFER_MAX);
	int lenU2G = BUFFER_MAX;
	YS_CodeConverter utf8ToGbk = YS_CodeConverter("utf-8","gbk");
	utf8ToGbk.Convert(*buffer, iUnzLen, cGbkBuf, lenU2G);

	printf("\n %s \n",cGbkBuf);
	printf("\n zhuang huan ceshi\n");
	memcpy(*buffer, cGbkBuf, strlen(cGbkBuf));
	printf(" _GBK ok \n");
#else
	#ifdef _GBK_TEST
	//char * cGbkBuf = (char*)malloc(BUFFER_MAX);
	char cGbkBuf[BUFFER_MAX];
	memset(cGbkBuf, 0, BUFFER_MAX);
	int lenU2G = BUFFER_MAX;
	YS_CodeConverter utf8ToGbk = YS_CodeConverter("utf-8","gbk");
	utf8ToGbk.Convert(*buffer, iUnzLen, cGbkBuf, lenU2G);
	
	printf("\n %s \n",cGbkBuf);
	printf("\n zhuang huan ceshi\n");
	//free(cGbkBuf);
	#endif
#endif

	return iUnzLen;
}

bool YS_Service::DbChecker()
{
	//数据库连接
	bool bRetStat = true;
	MYSQL   mysql;      
	mysql_init(&mysql);     
      
	if(mysql_real_connect(&mysql,m_DbHost.c_str(),m_userName.c_str(),m_passwd.c_str(),m_dbName.c_str(),0,NULL,0))     
        //if(mysql_real_connect(&mysql,NULL,m_userName.c_str(),m_passwd.c_str(),m_dbName.c_str(),0,NULL,0))
	{     
		bRetStat = true;
	}     
	else     
	{          
		bRetStat = false;     
	} 
	mysql_close(&mysql);
	return bRetStat;
}

int  YS_Service::InflateRead(char  *source,int  len,char  **dest, int *dest_size, int  gzip)
{
	int  iRetStat;
	unsigned  have;
	z_stream  strm;
	unsigned  char  out[CHUNK];
	int  totalsize  =  0;
 
	/*  allocate  inflate  state  */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;

	if(gzip)
		iRetStat = inflateInit2(&strm,  47);
	else
		iRetStat = inflateInit(&strm);

	if  (iRetStat  !=  Z_OK)
		return  iRetStat;

	strm.avail_in  =  len;
	strm.next_in = (unsigned char *)source;

	/*  run  inflate()  on  input  until  output  buffer  not  full  */
	do 
	{
		strm.avail_out = CHUNK;
		strm.next_out = out;
		iRetStat  =  inflate(&strm, Z_NO_FLUSH);
		if(iRetStat == Z_STREAM_ERROR) return Z_STREAM_ERROR;
		//assert(iRetStat != Z_STREAM_ERROR);
                   
		/*  state  not  clobbered  */
		switch  (iRetStat) 
		{
			case  Z_NEED_DICT:
				iRetStat  =  Z_DATA_ERROR;         
				/*  and  fall  through  */
			case  Z_DATA_ERROR:
			case  Z_MEM_ERROR:
				inflateEnd(&strm);
				return  iRetStat;
		}
 
		have  =  CHUNK  -  strm.avail_out;
		totalsize  +=  have;
		*dest  = (char *)realloc(*dest,totalsize);
		memcpy(*dest  +  totalsize  -  have,out,have);
	}  while  (strm.avail_out  ==  0);

	if (dest_size != NULL)
		*dest_size = strm.total_out;

	/*  clean  up  and  return  */
	(void)inflateEnd(&strm);
	return  iRetStat  ==  Z_STREAM_END  ?  Z_OK  :  Z_DATA_ERROR;
}
