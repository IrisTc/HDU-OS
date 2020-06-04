#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define RECEIVE_TYPE 2
#define SENDER_TYPE 1

//消息缓冲区
struct msgbuf{
    long mtype;           //消息类型（大于0的长整数）
    char mtext[1024];    //消息正文
};

int createMsgQueue();
int getMsgQueue();
int dstyMsgQueue(int msg_id);
int sendMsgQueue(int msg_id, int type, char* msg);
int recvMsgQueue(int msg_id, int recvType, char out[]);   
