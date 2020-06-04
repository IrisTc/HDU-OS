//列出系统所有内核线程的程序名、PID号、进程状态和进程优先级
//链表遍历操作  list.h
//

#define next_task(p) list_entry(rcu_dereference((p)->tasks.next), struct task_struct, tasks)

#define for_each_process(p) \
for (p = &init_task ; (p = next_task(p)) != &init_task ; )
/* list_for_each - iterate over a list
* @pos: the &struct list_head to use as a loop cursor.
* @head: the head for your list.
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init_task.h>
#include <linux/sched.h>

static int show_all_thread_init(void)
{
    int count=0;
    struct task_struct *p;
    printk(KERN_ALERT"%-23s%-6s%-8s%-8s%-8s\n","程序名","PID","状态","优先级","父进程PID");
    //线性遍历方式访问所有进程
    //
    /*    
    for_each_process是宏循环控制语句，在/include/linux/sched/signal.h中
    #define for_each_process(p) \
    for (p = &init_task ; (p = next_task(p)) != &init_task ; )
    其中next_task也是宏循环控制语句，在/include/linux/sched/signal.h中：
    #define next_task(p) \
    list_entry_rcu((p)->tasks.next, struct task_struct, tasks)
    而task_struct中有：
    struct task_struct {    
    ...
        struct list_head        tasks;
    ...
    }
    list_head：双向链表
*/
    //线性遍历方式访问该链表所有进程
    for_each_process(p)
    {
        //kernel thread是没有进程用户地址空间的，所以内核线程没有进程用户空间描述符，task->mm域是空（NULL）
        if(p->mm==NULL){
            printk(KERN_ALERT"%20s%6d%6d%6d%6d\n",
                p->comm, p->pid, p->state, p->prio, p->parent->pid);
            count++;
        }
    }
    printk("一共有:%d个内核线程\n",count);
    return 0;
}
static void show_all_thread_exit(void)
{
    printk(KERN_ALERT"show over\n");
}

//初始化，模块注册和申请资源
module_init(show_all_thread_init);
//退出，注销和释放资源
module_exit(show_all_thread_exit);
MODULE_LICENSE("GPL");