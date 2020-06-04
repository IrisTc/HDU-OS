

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>  
#include <semaphore.h>

#include <sys/types.h>


#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>


static const char * MUTEX_NAME = "mutex";
static const char * FULL_NAME  = "full";
//static const char * PATH_NAME = "tmp/shmtemp";


//constant define
#define SHM_SIZE 1024

void SemInit();

void SemDestroy();

void  P(sem_t *sem);

void  V(sem_t *sem);
