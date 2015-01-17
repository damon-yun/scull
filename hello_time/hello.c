#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>

struct timer_list mytimer;
struct mydata{
    int data;
};

void mytimer_interrupt(unsigned long data)
{
    struct mydata *prt = (struct mydata *)data;
    if(prt->data == 1)
	printk("Hello! This is timerinterrupt!\n");
}
//int mytime(struct inode * inode, struct file *filp)
int mytime(void)
{
    struct mydata *ptr;
    ptr =(struct mydata *) kzalloc(sizeof(struct mydata),GFP_KERNEL);
    if(ptr == NULL)return 0;
    ptr->data = 1;
    init_timer(&mytimer);
    mytimer.expires = 5*HZ;
    mytimer.function = mytimer_interrupt;
    mytimer.data = (unsigned long)ptr;
	add_timer(&mytimer);
    printk(KERN_ALERT "you have call timerinterrupt\n");
    //del_timer_sync(&mytimer);
    return 0; 
}
/*
struct file_operations scull_fops = {
        .owner = THIS_MODULE,
         .open = mytime,
};
*/
static int hello_init(void)
{
    printk(KERN_ALERT "Hello, world\n");
   // proc_create_data("mytest",0, NULL, &scull_fops, NULL);
    mytime();
    return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT "Goodbye, cruel world!\n");
}

MODULE_LICENSE("Dual BSD/GPL"); 


module_init(hello_init);
module_exit(hello_exit);
