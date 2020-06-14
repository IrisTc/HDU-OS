//
// Created by TC on 2020/6/5.
//

#ifndef OS_TEST5_FILESYS_H
#define OS_TEST5_FILESYS_H

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BLOCKSIZE 1024
#define SIZE 1024000
#define MAXOPENFILE 10          //同时打开最大文件数
#define MAXREADSIZE 10240
#define ROOTBLOCKNUM 2          //root占的磁盘块数量
#define END 65535
#define FREE 0
#define ATTR_DIR 0
#define ATTR_FILE 1

typedef struct BLOCK {
    char magic_number[8];       //魔数
    char information[200];      //文件系统信息
    unsigned short root;        //根目录起始盘块号
    unsigned char *startblock;  //虚拟磁盘数据区开始位置
} sysblock;

typedef struct FCB {
    char filename[8];
    char exname[3];
    unsigned char attribute;   //0目录文件，1数据文件
    unsigned short time;       //文件创建时间
    unsigned short date;       //文件创建日期
    unsigned short first;      //文件起始盘块号
    unsigned long length;      //文件大小
    char free;                 //0：空fcb
} fcb;

typedef struct FAT {
    unsigned short id;
} fat;

typedef struct USEROPEN {
    char filename[8];
    char exname[3];
    unsigned char attribute;   //0目录文件，1数据文件
    unsigned short time;       //文件创建时间
    unsigned short date;       //文件创建日期
    unsigned short first;      //文件起始盘块号
    unsigned long length;      //文件大小
    char free;                 //0：空fcb

    int dirno;                 //在父目录文件中的盘块号
    int diroff;                //fcb在父目录中的逻辑序号
    char dir[80];              //全路径信息
    int pos;                   //读写指针在文件中的位置
    char fcbstate;             //是否修改
    char topenfile;            //打开的表项是否为空
} useropen;

unsigned char *myhard;          //指向虚拟磁盘内存的起始地址
unsigned char buffer[SIZE];     //缓冲区
useropen filelist[MAXOPENFILE]; //用户打开文件表数组
int currfd;                     //用户当前目录位置
unsigned char *startb;          //记录虚拟磁盘数据区开始位置

void initSys();
void my_exitsys();
int do_write(int fd, char *text, int len, char wstyle);
int do_read(int fd, int len, char *text);
void my_format();
int my_cd(char *dirname);
void my_ls();
void my_mkdir(char *dirname);
void my_rmdir(char *dirname);
int my_create(char *filename);
void my_rm(char *filename);
int my_open(char *filename);
int my_close(int fd);
int my_read(int fd);
int my_write(int fd);

int get_free_block();
int get_free_filelist();
void _copy();


#endif //OS_TEST5_FILESYS_H
