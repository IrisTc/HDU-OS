#include "comm.h"

int main()
{
    //互斥读写消息队列数据,初值为1
    sem_t *mutex = sem_open("mutex", O_CREAT|O_RDWR, 0644, 1);
    //空缓冲区和满缓冲区数量
    //sender发送消息必须有：有空闲的消息缓冲区；消息队列当前状态空闲
    //receiver接受消息必须有：消息缓冲区中有没读过的数据；消息队列当前状态空闲
    sem_t *empty = sem_open("empty", O_CREAT|O_RDWR, 0644, 3);
    sem_t *full = sem_open("full", O_CREAT|O_RDWR, 0644, 0);
    //防止receiver的回应还没给sender，sender2发送消息
    sem_t *resp1 = sem_open("resp1", O_CREAT|O_RDWR, 0644, 1);
    sem_t *resp2 = sem_open("resp2", O_CREAT|O_RDWR, 0644, 1);

    int msgid = createMsgQueue();
    char buf[1024] = {0};
    int count = 0;
    sem_wait(resp1);
    sem_wait(resp2);
    while(1)
    {
        sem_wait(full);         //receiver等待消息队列缓冲区中有消息
        sem_wait(mutex);        //申请消息队列读写资源
        recvMsgQueue(msgid, SENDER_TYPE, buf);
        printf("sender: %s\n", buf);
        sem_post(mutex);        //释放消息队列读写资源
        sem_post(empty);        //释放一个缓冲区
        if(strcasecmp("end1", buf) == 0){
            char response[8] = "over1";
            sem_wait(mutex);
            sendMsgQueue(msgid, RECEIVE_TYPE, response);
            sem_post(mutex);
	    sem_post(resp1);      //释放回应写入信号
            
            count++;
	    printf("%d",count);
        }
        if(strcasecmp("end2", buf) == 0){
            char response[8] = "over2";
            sem_wait(mutex);
            sendMsgQueue(msgid, RECEIVE_TYPE, response);
            sem_post(mutex);
            sem_post(resp2);
            sem_post(full);
            count++;
        }
        if(count == 2){
            break;
        }
    }
    sem_close(mutex);
    sem_close(empty);
    sem_close(full);
    sem_close(resp1);
    sem_close(resp2);
    sem_unlink("mutex");
    sem_unlink("empty");
    sem_unlink("full");
    sem_unlink("resp1");
    sem_unlink("resp2");
    dstyMsgQueue(msgid);
    return 0;
}
