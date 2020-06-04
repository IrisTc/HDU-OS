//头文件声明
#include <linux/init.h>  //模块初始化和清理函数的定义
#include <linux/module.h>  //加载模块所需要的的函数和符号定义
#include <linux/kernel.h>  //printk函数需要


//内核模块没有main函数，必须定义两个函数
//初始化函数用来完成模块注册和申请资源
//static使这个函数不会在特定文件之外可见
//如果只是初始化使用一次的话可以在生命语句中加__init标识，则模块加载后会丢弃释放其内存空间
static int hello_init(void)
{
    printk(KERN_ALERT"hello,world\n");
    return 0;
}

//退出函数用来完成注销和释放资源
static void hello_exit(void)
{
    printk(KERN_ALERT"goodbye\n");
}

module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");  //宏生命此模块的许可证 GNU General Public License