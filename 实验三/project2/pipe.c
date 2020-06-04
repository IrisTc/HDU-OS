#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define MSG1_SIZE 10000

int main(){
    int fd[2];
    ssize_t n;

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
         *管道默认是阻塞写，通过`fcntl`设置成非阻塞写，在管道满无法继续写入时返回-EAGAIN，作为循环终止条件         */
        int count = 0;
        char msg1[MSG1_SIZE];
        memset(msg1, 0, MSG1_SIZE)
        while(1){
            n = write(fd[1], msg1, MSG1_SIZE);
            if(n = -1){
                break;
            }else {
                count++;
                printf("Child1 write %dB\n", n)
            }
        }
        
        exit(0);
    }

    //子进程2
    pid_t pid2 = fork();
    if(pid2<0){
        printf("创建子进程失败\n");
    }
    else if(pid2==0){
        printf("Child2 is writing...\n");
        close(fd[0]);
        char *msg2 = "我是子进程2写入的数据\n";
        write(fd[1], msg2, strlen(msg2));
        exit(0);
    }

    //子进程3
    pid_t pid3 = fork();
    if(pid3<0){
        printf("创建子进程失败\n");
    }
    else if(pid3==0){
        printf("Child3 is writing...\n");
        close(fd[0]);
        char *msg3 = "我是子进程3写入的数据";
        write(fd[1], msg3, strlen(msg3));
        exit(0);
    }

    //三个子进程写完，父进程开始读入
    else{
        //参数status如果是一个空指针,则表示父进程不关心子进程的终止状态
        //只等待进程ID等于pid的子进程，不管其它已经有多少子进程运行结束退出了，只要指定的子进程还没有结束，waitpid就会一直等下去
        waitpid(pid1, NULL, WUNTRACED);
        waitpid(pid2, NULL, WUNTRACED);
        waitpid(pid3, NULL, WUNTRACED);
        printf("father is reading...\n");
        close(fd[1]);
        char result[100];
        read(fd[0], result, sizeof(result));
        printf("Children say:\n%s\n", result);

    }
    return 0;
}