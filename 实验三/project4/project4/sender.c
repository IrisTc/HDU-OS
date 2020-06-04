#include "common.h"

//创建共享内存函数shmget(key_t key，int size,int shmflg);
key_t key;  

//shared memory
int shmid;
char * shmptr;   //shmat()函数返回的映射后的地址
char input[SHM_SIZE];

//semaphore 
sem_t * full;   //同步信号量
sem_t * mutex;  //互斥信号量

int main(int argc, char *argv[])
{
	//初始化信号量
	SemInit();
	full = sem_open(FULL_NAME,O_CREAT,0644,0);
	mutex = sem_open(MUTEX_NAME,O_CREAT,0644,1);
	resp = sem_open("resp",O_CREAT，0644,0);

	//创建key值
	key = keyInit();

	// 创建共享内存
	shmid = shmidGet(key);

	// 将共享内存映射到用户空间
	shmptr = shmptrGet(shmid);


	//等待用户输入
	scanf("Please input: %s",input);

	//发送消息
	P(mutex);						
	strcpy(shmptr,input);    
	V(mutex);
	V(full);

    P(resp);
	P(mutex);						
	strcpy(input,shmptr);    
	V(mutex);
	printf("response: %s\n", input);

	return 0;
}