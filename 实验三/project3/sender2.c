#include "comm.h"

int main()
{
    //互斥读写消息队列数据,初值为1
    sem_t *mutex = sem_open("mutex", O_CREAT|O_RDWR, 0644, 1);
    //空消息队列和满消息队列数量
    //sender发送消息必须有：有空闲的消息缓冲区；消息队列当前状态空闲
    //receiver接受消息必须有：消息缓冲区中有没读过的数据；消息队列当前状态空闲
    sem_t *empty = sem_open("empty", O_CREAT|O_RDWR, 0644, 3);
    sem_t *full = sem_open("full", O_CREAT|O_RDWR, 0644, 0);
    //防止receiver的回应还没给sender，sender2发送消息
    sem_t *resp1 = sem_open("resp1", O_CREAT|O_RDWR, 0644, 1);
    sem_t *resp2 = sem_open("resp2", O_CREAT|O_RDWR, 0644, 1);

    int msgid = getMsgQueue();
    char buf[1024] = {0};
    while(1)
    {
        printf("Please input: ");
        fflush(stdout);
        ssize_t s = read(0, buf, sizeof(buf));
        if(s > 0)
        {
            buf[s-1]=0;
            sem_wait(empty);          //申请消息缓冲区资源
            sem_wait(mutex);          //申请消息队列读写资源
            if(strcasecmp("exit", buf) == 0){
                strcpy(buf, "end2");
                sendMsgQueue(msgid, SENDER_TYPE, buf);
                printf("send done, I will exit...\n");
                sem_post(mutex);      //释放消息队列读写资源
                sem_post(full);       //释放消息队列写入了数据信号
                //resp防止sender和receiver竞争full
		sem_wait(resp2);
                recvMsgQueue(msgid, RECEIVE_TYPE, buf);
                printf("response: %s\n", buf);
                sem_post(mutex);
		sem_post(empty);
                break;
            }else{
                sendMsgQueue(msgid, SENDER_TYPE, buf);
                printf("send done, wait recv...\n");
                sem_post(mutex);
                sem_post(full);
            }
        }
    }
    return 0;
}
