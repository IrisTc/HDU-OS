#include "common.h"

//key
key_t key;

//shared memory
int shmid;
char * shmptr;
char result[SHM_SIZE];

//semaphore 
sem_t * full;
sem_t * mutex;

int main(int argc, char *argv[])
{
	//初始化信号量
	SemInit()
	full = sem_open(FULL_NAME,O_CREAT,0644,0);
	mutex = sem_open(MUTEX_NAME,O_CREAT,0644,1);
	resp = sem_open("resp",O_CREAT,0644,0);

	//创建key值
	key = keyInit();
	
	//打开共享内存
	shmid = shmIdGet(key); 
	
	// 将共享内存映射到用户空间
	shmptr = shmptrGet(shmid);	

	//读共享内存区数据
	P(full);
	P(mutex);						
	strcpy(result,shmptr);
	V(mutex);
	printf("Receiver : message is %s\n",result);

	P(mutex);						
	strcpy(shmptr, "over");
	V(mutex);
	V(resp);
	

	shmDestroy(shmid, shmptr);
	SemDestroy();
	return 0;
}