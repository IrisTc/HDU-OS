#include "common.h"

int keyInit()
{
	key = ftok("../", 2015); 
	if(key == -1)
	{
		perror("ftok");
	}
	return key;
}

int shmidGet(key)
{
	shmid = shmget(key, SHM_SIZE, IPC_CREAT|0666);	
	if(shmid < 0) 
	{ 
		perror("shmget"); 
		exit(-1); 
	}
	return shmid;
}

char *shmptrGet(shmid)
{
	char *shmptr = shmat(shmid,NULL,0);		
	if(shmptr < 0)
	{
		perror("shmat");
		exit(-1);
	}
	return shmptr;
}

void shmDestroy(shmid, shmptr)
{
	//分离共享内存和当前进程
	if(shmdt(shmptr) < 0)
	{
		perror("shmdt");
		exit(1);
	}
	else
	{
		printf("deleted shared-memory\n");
	}
	
	//删除共享内存
	shmctl(shmid, PC_RMID, NULL);
}

void SemInit()//创建信号量
{
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


void SemDestroy()
{
	sem_t * mutexPtr = sem_open(MUTEX_NAME,O_CREAT,0644,1); 
	sem_t * fullPtr= sem_open(FULL_NAME,O_CREAT,0644,0);

	
	sem_close(mutexPtr);			// int sem_close(sem_t *sem);
	sem_unlink(MUTEX_NAME);			// int sem_unlink(const char *name);

	
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