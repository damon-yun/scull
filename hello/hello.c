#include <linux/init.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <asm/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/types.h>

int  printf_major = 0,printf_minor = 0;
int  printf_dev_num = 1;

struct printf_dev{
    struct cdev cdev;
    int size;
    char *data;
} printf_dev;

int printf_trim(struct printf_dev *dev)
{
    if(dev){
        if(dev->data){
            kfree(dev->data);
            dev->data = NULL;
            dev->size = 0;
        }
    }
}

loff_t printf_lseek(struct file *filp,loff_t offset,int orig)
{
    struct printf_dev *dev = filp->private_data;
    int ret;
    switch (orig){
    case 0:
        ret = offset;
        break;
    case 1:
        ret = filp->f_pos + offset;
        break;
    case 2:
        ret = dev->size + offset;
        break;
    default :
        return -EINVAL;
    }
    filp->f_pos = ret;
    return ret;
}

int printf_open(struct inode * inode, struct file *filp)
{
    struct printf_dev *dev = container_of(inode->i_cdev,struct printf_dev,cdev);
    filp->private_data = dev;
    if ((filp->f_flags & O_ACCMODE) == O_WRONLY)
    {
      //  if (down_interruptible(&pdev->sem))
        //                                return -ERESTARTSYS;
        printf_trim(dev);
          //                              up(&pdev->sem);
    }
    return 0;
}

ssize_t printf_write(struct file *filp, const char __user *buf, size_t count,
                        loff_t *f_pos)
{
    struct printf_dev *dev = filp->private_data;
    if(dev->data == NULL){
        dev->data = (char *)kmalloc(dev->size*sizeof(char),GFP_KERNEL);
        if(!dev->data){
            return -ENOMEM;
        }
        memset(dev->data,0,count*sizeof(char));
    }
    if(*f_pos>dev->size){
        return -ENOMEM;
    }
    if(count>(dev->size-*f_pos)){
        count = dev->size-*f_pos;
    }
    if (copy_from_user(dev->data + *f_pos,buf,count))
    {
        return  -EFAULT;
    }
    *f_pos += count;
    return count;
}

ssize_t printf_read(struct file *filp, const char __user *buf, size_t count,
                     loff_t *f_pos)
{
    struct printf_dev *dev = filp->private_data;
    if(dev->data == NULL){
        return 0;
    }
    if(*f_pos > dev->size){
        return 0;
    }
    if(count > dev->size-*f_pos){
        count = dev->size-*f_pos;
    }
    if(copy_to_user(buf,dev->data + *f_pos,count)){
        return 0;
    }
    return count;
}


int printf_close(struct inode * inode, struct file *filp)
{
    return 0;
}

struct file_operations printf_fops = {
        .owner = THIS_MODULE,
        .open  = printf_open,
        .write = printf_write,
        .release = printf_close,
        .llseek = printf_lseek,
};



int printf_dev_setup(struct printf_dev *dev)
{
    int err;
    int dev_id = MKDEV(printf_major,printf_minor);
    cdev_init(&dev->cdev,&printf_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops   = &printf_fops;
    err = cdev_add(&dev->cdev,dev_id,printf_dev_num);
    if(err){
        printk(KERN_WARNING "printf_dev can't get dev_id\n");
        kfree(dev->data);
        return err;
    }
    return 0;
}

static int printf_init(void)
{
    dev_t dev = 0;
    int err;
    if(printf_major){
        dev = MKDEV(printf_major,printf_minor);
        err = register_chrdev_region(dev,printf_dev_num,"myprintf");
    } else {
        err =alloc_chrdev_region(&dev,printf_minor,printf_dev_num,"myprintf");
        printf_major = MAJOR(dev);
    }
    if(err < 0){
        printk(KERN_WARNING "myprintf can't get major.err = %d\n",err);
        return err;
    }
    err = printf_dev_setup(&printf_dev);
    if(err){
        printk(KERN_WARNING "cdev_add fail\n");
        goto err_cdev_add;
    }

    printk(KERN_WARNING "myprintf init ok!\n");
    return 0;

err_cdev_add:
    unregister_chrdev_region(dev,printf_dev_num);

    return err;
}

static void printf_exit(void)
{
    int dev = MKDEV(printf_major,printf_minor);
    cdev_del(&printf_dev.cdev);
    kfree(printf_dev.data);
    unregister_chrdev_region(dev,printf_dev_num);
	printk(KERN_ALERT "Goodbye, cruel world!\n");
}

MODULE_LICENSE("Dual BSD/GPL"); 


module_init(printf_init);
module_exit(printf_exit);
