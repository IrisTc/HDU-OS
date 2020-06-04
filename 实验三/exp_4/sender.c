#include "common.h"

//创建共享内存函数shmget(key_t key，int size,int shmflg);
key_t key;  

//shared memory
int shmid;
char * shmptr;   //shmat()函数返回的映射后的地址
char input[SHM_SIZE];

//semaphore 
sem_t * full;
sem_t * mutex;
//end of semaphore


void Init()
{
	key = KEY_NUM;					//初始化key
	shmid  = GetShmId(key);			// 初始化shared memory
	shmptr = shmat(shmid,NULL,0);		// 将共享内存映射到用户空间
	//初始化信号量
	full = sem_open(FULL_NAME,O_CREAT);
	mutex = sem_open(MUTEX_NAME,O_CREAT);
}

void SaveMessage()  //从读取的字符串中获取内容
{

	P(mutex);						
	strcpy(shmptr,input);
	V(mutex);

	V(full);
}

int main(int argc, char const *argv[])//读取字符串和调用主要函数
{
	
	
	Init();
	
	/*waiting for user to input message*/
	scanf("%s",input);		//input message from shell 
							// TODO input a whole line
	SaveMessage();
	
	printf("Sender:  Process End\n");
	return 0;
}