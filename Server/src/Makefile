#System Definition For Softswitch
SSDIR=$(HOME)/Server/src#可能的工程代码目录

#System LIB Definition
SYSNLIB= -lz -lbz2 -lpthread#可能要链接的系统链接库

# The following for SUN Forte C++
# CC Definition
#CC=CC                #Solaris的编译器

# The following for gcc
CC=g++#LINUX的编译器

# ----------------------------------------
# The following lines are only probably 
# needed to be modified.
# Path Setting

CFLAGS=-g -I$(SSDIR) -I/usr/include/mysql -I. -I/usr/include #-D_GBK
#可能的编译选项(options) 
#当程序当中使用了命名空间时，才加上-AA选项，否则编译有可能出错

BINPATH = $(HOME)/Server/bin#生成可执行文件的路径

# All Source Files
SRC=YsThreadJob.C YsThreadWorker.C YsThreadPoolManager.C YsThreadPool.C \
	YsTools.C YsFileop.C YsSocketop.C YsService.C YsServiceJob.C main.C 
                                               #要编译的原文件列表
# EXE File
EXE = $(BINPATH)/YS_urlgetor#生成的可执行文件名
# Please modify the above if needed, and
# do not change the other lines in this file.
#-------------------------------------------

# For ORACLE    数据库目录、链接库目录。
MYSQL_HOME=/var/lib/mysql/mysql
MYSQL_LIB_PATH=-L$(MYSQL_HOME)/lib64 -L/usr/lib64/mysql
MYSQL_LIB=-lmysqlclient

# LIB = System LIB add Softswitch LIB   将各连接库目录归总
LIBPATH= $(MYSQL_LIB_PATH) -L/usr/lib64

LIB=  $(SYSNLIB) $(MYSQL_LIB)#将各链接库归总

# All OBJ Files   目标文件，参考make后缀规则
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
