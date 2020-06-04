#include "common.h"

int GetShmId(key_t key)//定义操作共享内存的函数
{
	int shmid;

	shmid = shmget(key,SHM_SIZE,IPC_CREAT|0666);//创建一个共享内存对象
	if(shmid < 0)//如果不成功显示错误信息
	{
		perror("Receiver: Shmget Error");
		exit(EXIT_FAILURE);
	}

	return shmid;
}

/*
* create mutex + semaphore
* init those value
*/
void SemInit()//创建信号量
{
	 /*
	 * Funtion Prototype: 
	 *
	 *	sem_t *sem_open(const char *name, int oflag,
	 *                  mode_t mode, unsigned int value);
	 *                 
	 * name 	: MUTEX_NAME 	"mutex_shm"
	 * oflag	: O_CREAT 		Create and initialize it if not exist
	 * mode_t	: file perssion -rw-r--r--
	 * value 	: 1
	 */
	 if((sem_open(MUTEX_NAME,O_CREAT,0644,1)) < 0)
	 {
	 	perror("sem_open");
	 	exit(EXIT_FAILURE);
	 }

	 if((sem_open(FULL_NAME,O_CREAT,0644,0)) < 0){
	 	perror("sem_open");
	 	exit(EXIT_FAILURE);
	 }
}


/*
* close and unlink semaphore that we crated
*/
void SemDestroy()
{
	sem_t * mutexPtr = sem_open(MUTEX_NAME,O_CREAT); 
	sem_t * fullPtr= sem_open(FULL_NAME,O_CREAT);

	/* Destroy mutex */
	sem_close(mutexPtr);				// int sem_close(sem_t *sem);
	sem_unlink(MUTEX_NAME);			// int sem_unlink(const char *name);

	/* Destory full*/
	sem_close(fullPtr);
	sem_unlink(FULL_NAME);
}


void  P(sem_t *semPtr)
{
	sem_wait(semPtr);					//int sem_wait(sem_t *sem);
}

void  V(sem_t *semPtr)
{
	sem_post(semPtr);					//int sem_post(sem_t *sem);
}