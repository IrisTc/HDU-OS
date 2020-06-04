#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

char* option[4] = {"hello", "five", "bye", "exit"};

int toNum(char *str){
    for (int i=0; i<5; i++){
        if(strcmp(str, option[i]) == 0){
            return i;
        }
    }
    return -1;
}

int create_child(int opNum){
    int status = -1;
    pid_t fpid;
    fpid=fork();
    if(fpid<0){
        printf("创建进程失败");
        return -1;
    }
    //子进程
    else if(fpid==0){
        switch(opNum){
            case 0:
            status = execl("./cmd1", "hello",  NULL);
            break;
            case 1:
            status = execl("./cmd2", "five",  NULL);
            break;
            case 2:
            status = execl("./cmd3", "bye",  NULL);
            break;
            case 3:
            printf("exit\n");
            break;
            default:
            printf("找不到该选项");
            break;
        }
    }
    //父进程
    else{            //返回子进程pid
        wait(&status);
        printf("愉快的对话结束了！（父进程）\n");
    }
}

int main()
{
    char op[5];
    int opNum;
    while(opNum!=3){
        printf("choose an option:\n (hello) 打招呼(*^▽^*)  （five）击掌(✧∇✧)╯╰(✧∇✧)̣  （bye）再见ヽ(ー_ー)ノ  （exit）退出\n");
        scanf("%s", op);
        opNum = toNum(op);
        create_child(opNum);
    }
    return 0;
}