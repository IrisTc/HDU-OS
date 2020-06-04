#include "comm.h"
#define KEY_VALUE 0424

int commMsgQueue(int flags){
    //创建和访问一个消息队列
    //调用成功返回一个key值，用于创建消息队列，如果失败，返回-1
    key_t key = ftok("./", KEY_VALUE);
    //成功返回一个非负整数，即消息队列的标识码，失败返回-1;
    int msg_id = msgget(key, flags);//获得消息队列的特征标识符,key该消息队列的全局唯一标识符，
                                    //通过该标识符可以定位消息队列，对其进行相关操作
    if(msg_id < 0){
        //perror:把一个描述性错误消息输出到标准错误 stderr
        perror("消息队列错误：");
        exit(-1);
    }
    return msg_id;
}

int createMsgQueue(){
    return commMsgQueue(IPC_CREAT | IPC_EXCL | 0666);
}

int getMsgQueue(){
    return commMsgQueue(IPC_CREAT);
}

int dstyMsgQueue(int msg_id){
    //消息队列的控制函数，IPC_RMID：删除消息队列
    if(msgctl(msg_id, IPC_RMID, NULL)<0){
        perror("msgctl");
        return -1;
    }
    return 0;
}

int sendMsgQueue(int msg_id, int type, char *msg){
    struct msgbuf buf;
    buf.mtype = type;
    strcpy(buf.mtext, msg);
    //把一条消息添加到消息队列中
    //是一个消息发送队列
    if(msgsnd(msg_id, &buf, sizeof(buf.mtext), 0)<0){
        perror("msgsnd");
        return -1;
    }
    return 0;
}

int recvMsgQueue(int msg_id, int recvType, char out[]){
    struct msgbuf buf;
    int size = sizeof(buf.mtext);
    //是从一个消息队列接受消息
    //成功返回实际放到接收缓冲区里去的字符个数，失败返回-1
    //msg_id 是函数 msgget 的返回值，用于表示对哪一个消息队列进行操作。
    //buf是接收消息的指针，指向消息结构体
    //size 是接收消息的大小，这里可以看作结构体中数据段的大小
    if(msgrcv(msg_id, &buf, size, recvType, 0)<0){   
        perror("msgrcv");
        return -1;
    }
    strncpy(out, buf.mtext, size);
    out[size] = 0;
    return 0;
}

