#System Definition For Softswitch
SSDIR=$(HOME)/Server/src#���ܵĹ��̴���Ŀ¼

#System LIB Definition
SYSNLIB= -lz -lbz2 -lpthread#����Ҫ���ӵ�ϵͳ���ӿ�

# The following for SUN Forte C++
# CC Definition
#CC=CC                #Solaris�ı�����

# The following for gcc
CC=g++#LINUX�ı�����

# ----------------------------------------
# The following lines are only probably 
# needed to be modified.
# Path Setting

CFLAGS=-g -I$(SSDIR) -I/usr/include/mysql -I. -I/usr/include #-D_GBK
#���ܵı���ѡ��(options) 
#��������ʹ���������ռ�ʱ���ż���-AAѡ���������п��ܳ���

BINPATH = $(HOME)/Server/bin#���ɿ�ִ���ļ���·��

# All Source Files
SRC=YsThreadJob.C YsThreadWorker.C YsThreadPoolManager.C YsThreadPool.C \
	YsTools.C YsFileop.C YsSocketop.C YsService.C YsServiceJob.C main.C 
                                               #Ҫ�����ԭ�ļ��б�
# EXE File
EXE = $(BINPATH)/YS_urlgetor#���ɵĿ�ִ���ļ���
# Please modify the above if needed, and
# do not change the other lines in this file.
#-------------------------------------------

# For ORACLE    ���ݿ�Ŀ¼�����ӿ�Ŀ¼��
MYSQL_HOME=/var/lib/mysql/mysql
MYSQL_LIB_PATH=-L$(MYSQL_HOME)/lib64 -L/usr/lib64/mysql
MYSQL_LIB=-lmysqlclient

# LIB = System LIB add Softswitch LIB   �������ӿ�Ŀ¼����
LIBPATH= $(MYSQL_LIB_PATH) -L/usr/lib64

LIB=  $(SYSNLIB) $(MYSQL_LIB)#�������ӿ����

# All OBJ Files   Ŀ���ļ����ο�make��׺����
OBJ=\
	$(SRC:.C=.o)
# Begin To Make.	  
all :$(EXE)

$(EXE):$(OBJ) $(SRC)
	$(CC) -o $(EXE) $(CFLAGS) $(LIBPATH) $(OBJ) $(LIB)
	echo         $(EXE) ok

.C.o:
	$(CC) -c -o $@ $(CFLAGS)  $<

# Clean the EXE And OBJ Files made just now.
clean:
	rm $(EXE) $(OBJ)