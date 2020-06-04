
//分配系统调用号  arch/x86/entry/syscalls/syscall_64.tbl
333    common    mysetnice    sys_mysetnice





//申请系统调用服务例程原型   include/linux/syscalls.h
asmlinkage long sys_mysetnice(pid_t pid, int flag, int nicevalue, void __user* prio, void __user* nice);

/**
 * 函数原型：int mysetnice(pid_t pid, int flag, int nicevalue, void __user * prio, void __user * nice);
 * pid: 进程 ID
 * flag: 若值为0，读取nice值；若值为1，修改nice值为nicevalue
 * nice,flag: 当前进程nice值和优先级
 * 返回值：系统调用成功返回0，否则返回错误码 EFAULT
 *
 * 宏“SYSCALL_DEFINEN(sname)”对服务例程原型进行了封装，其中的“N”是该系统调用所需要参数的个数
 */
SYSCALL_DEFINE5(mysetnice, pid_t, pid, int, flag, int, nicevalue, void __user*, prio, void __user*, nice)
{
    struct pid *ppid;                      //进程描述符指针，指向一个枚举类型
    struct task_struct *task;             //任务描述符信息
    ppid = find_get_pid(pid);              //通过索引pid_t返回pid
    task = pid_task(ppid, PIDTYPE_PID);    //通过pid返回进程task

    int curr_nice;
    curr_nice = task_nice(task);   //返回当前进程的nice值
    int curr_prio;
    curr_prio = task_prio(task);   //返回当前进程的prio值

    if(flag == 1){           //如果flag等于1修改进程的nice值   
        set_user_nice(task, nicevalue);
        curr_nice = task_nice(task);      //重新获取修改过的nice值
        curr_prio = task_prio(task);      //重新获取修改过的prio值    
    }
    else if(flag != 0){      //如果flag不等于1或者0返回错误码
        return EFAULT;
    }

    copy_to_user(nice, &curr_nice, sizeof(curr_nice));    //将nice值拷贝到用户空间
    copy_to_user(prio, &curr_prio, sizeof(curr_prio));    //将prio值拷贝到用户空间
    return 0;
}






//测试代码
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>

#define _SYS_TSSETNICE_ 333
#define EFAULT 14

int main()
{
    int pid, flag, nicevalue;
    int prev_prio, prev_nice, curr_prio, curr_nice;
    int result;

    printf("Hello, this is TS's syscall test...");
    printf("Please input variable like this: pid, flag(0/1), nicevalue");
    scanf("%d%d%d", &pid, &flag, &nicevalue);

    result = syscall(_SYS_TSSETNICE_, pid, 0, nicevalue, &prev_prio, &prev_nice);

    if(result == EFAULT){
        printf("ERROR!\n");
        return 1;
    }
    else if(flag == 1){
        syscall(_SYS_TSSETNICE_, pid, 1, nicevalue, &prev_prio, &prev_nice);
        printf("Original priority is %d, Original nice is %d\n", prev_prio, prev_nice);
        printf("Current priority is %d, Current nice is %d\n", curr_prio, curr_nice);
    }
    else if(flag == 0){
        printf("Current priority is %d, Current nice is %d\n", curr_prio, curr_nice);
    }
    else{
        printf("flag is not exist\n");
    }
    return 0;
}