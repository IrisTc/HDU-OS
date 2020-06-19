#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <semaphore.h>

#define MSG1_SIZE 8192


int main(){
    int fd[2];
    ssize_t n;
    sem_t *write_mutex;
    sem_t *read_mutex1;
    sem_t *read_mutex2;
    write_mutex = sem_open("write", O_CREAT | O_RDWR, 0666, 0);
    read_mutex1 = sem_open("read1", O_CREAT | O_RDWR, 0666, 0);
    read_mutex2 = sem_open("read2", O_CREAT | O_RDWR, 0666, 0);

    if(pipe(fd)!=0){
        printf("创建管道失败\n");
        exit(1);
    }
    //fork函数会将父进程的相关数据结构继承到子进程中
    //这样就使子进程中的文件描述符表中的fd[0]和fd[1]指向父进程所指向的管道文件,就能实现两个进程之间的通信
    //子进程1
    pid_t pid1 = fork();
    if(pid1<0){
        printf("创建子进程失败\n");
    }
    else if(pid1==0){
        printf("Child1 is writing...\n");
        close(fd[0]);

        int flags = fcntl(fd[1], F_GETFL, 0);
        fcntl(fd[1], F_SETFL, flags | O_NONBLOCK);
        /* 通过fcntl获取当前描述符fd的文件状态标记，
         * 然后将之与非阻塞标志O_NONBLOCK进行或操作再进行设置；
         *管道默认是阻塞写，通过fcntl设置成非阻塞写，在管道满无法继续写入时返回-EAGAIN，作为循环终止条件         */
        int count = 0;
        int flag = 1;
        char msg1[MSG1_SIZE];
        memset(msg1, 0, MSG1_SIZE);
        while(flag){
            n = write(fd[1], msg1, MSG1_SIZE);
            if(n == -1){
                flag = 0;
            }else {
                count++;
                printf("Child1 write %ldB\n", n);
            }
        }
        printf("total space = %dKB\n", (count * MSG1_SIZE) / 1024);
        exit(0);
    }

    //子进程2
    pid_t pid2 = fork();
    if(pid2<0){
        printf("创建子进程失败\n");
    }
    else if(pid2==0){
        sem_wait(write_mutex);
        printf("Child2 is writing...\n");
        close(fd[0]);
        write(fd[1], "我是子进程2写入的数据\n", 40);
        sem_post(write_mutex);
        sem_post(read_mutex1);
        exit(0);
    }

    //子进程3
    pid_t pid3 = fork();
    if(pid3<0){
        printf("创建子进程失败\n");
    }
    else if(pid3==0){
        sem_wait(write_mutex);
        printf("Child3 is writing...\n");
        close(fd[0]);
        write(fd[1], "我是子进程3写入的数据\n", 40);
        sem_post(write_mutex);
        sem_post(read_mutex2);
        exit(0);
    }

    //三个子进程写完，父进程开始读入
    //参数status如果是一个空指针,则表示父进程不关心子进程的终止状态
        //只等待进程ID等于pid的子进程，不管其它已经有多少子进程运行结束退出了，只要指定的子进程还没有结束，waitpid就会一直等下去
        //pid1 = waitpid(pid1, NULL, WUNTRACED);
        //pid2 = waitpid(pid2, NULL, WUNTRACED);
        //pid3 = waitpid(pid3, NULL, WUNTRACED);


    //父进程必须接收到子进程结束之后返回的 0，才能继续运行，否则阻塞。     
    //读取子进程一写入的数据，否则子进程二、三无法继续写入.     
    //读空管道后结束循环，释放信号量，子进程二、三继续运行
    wait(0);
    printf("Father is reading...\n");
    close(fd[1]);
    int flags = fcntl(fd[0], F_GETFL, 0);
    fcntl(fd[0], F_SETFL, flags | O_NONBLOCK);
    int flag = 1;
    char result[MSG1_SIZE];
    while(flag){
        n = read(fd[0], result, MSG1_SIZE);
        if(n == -1){
            flag = 0;
        }else {
            printf("Father read(非阻塞读): %ldB\n", n);
        }
    }

    fcntl(fd[0], F_SETFL, flags | ~O_NONBLOCK);// 设置阻塞性读，作为循环结束标志     
    fcntl(fd[1], F_SETFL, flags | ~O_NONBLOCK);// 设置阻塞性写
    
    sem_post(write_mutex);//子进程二、三继续写入
    sem_wait(read_mutex1);
    sem_wait(read_mutex2);

    printf("Father read(阻塞读):");  
    n = read(fd[0], result, MSG1_SIZE);
    printf("read %ldB\n", n);
    for (int i = 0; i < n; i++) {
        printf("%c", result[i]);
    }
    
    sem_close(write_mutex);
    sem_close(read_mutex1);
    sem_close(read_mutex2);
    sem_unlink("write");
    sem_unlink("read1");
    sem_unlink("read2");
    return 0;
}