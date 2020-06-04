//设计一个带参数的模块，参数为某个进程PID号，功能是列出该进程的家族信息，包括父进程、兄弟进程和紫禁城的程序名和PID号
#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include <linux/sched.h>
#include <linux/moduleparam.h>

static pid_t pid=1;
module_param(pid,int,0644);

//写的一个快速输出进程的信息的函数
void info_print(struct task_struct* p, char* description){
    printk("%-10s\tpid: %-9d 进程名: %-20s 进程状态: %ld\n",
        description,p->pid, p->comm, p->state);
}

static int show_family_init(void)
{
    struct pid *ppid;
    struct task_struct *p;
    struct list_head *pos;
    struct task_struct *temp_p;

    //打印自己
    ppid = find_get_pid(pid);
    if(ppid == NULL){
        printk("该PID不存在.");
        return -1;
    }
    p = pid_task(ppid, PIDTYPE_PID);
    info_print(p, "自身信息");

    // 父进程
    if(p->parent == NULL) {
        printk("无父进程\n");
    }
    else {
        info_print(p->parent, "父进程");
    }


    /**
    * list_for_each - iterate over a list
    * @pos: the &struct list_head to use as a loop cursor.
    * @head: the head for your list.
    #define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)
   */
    list_for_each(pos, &p->parent->children)
    {
        //从结构体（type）某成员变量（member）指针（ptr）来求出该结构体（type）的首指针
        temp_p = list_entry(pos, struct task_struct, sibling);
        //排除自己
        if (temp_p != p)
        {
            info_print(temp_p, "兄弟进程");
        }
    }

    // 子进程
    list_for_each(pos, &p->children)
    {
        temp_p = list_entry(pos, struct task_struct, sibling);
        info_print(temp_p, "子进程");
    }

    return 0;
}

static void show_family_exit(void)
{
    printk(KERN_ALERTs"show family over\n");
}

module_init(show_family_init);
module_exit(show_family_exit);

MODULE_LICENSE("GPL");