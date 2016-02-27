#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>		//cdev
#include <linux/semaphore.h> 	//信号量
#include <linux/slab.h>		//kzalloc
#include <linux/errno.h>	//error code
#include <linux/types.h>	//size_t
#include <asm/uaccess.h>	//copy_*_user


#include "detector.h"


int detector_minor = MY_MAJOR;
int detector_major = 0;


struct detector_dev *detector=NULL;


extern int do_detect(struct detector_dev *detector,int count,char __user * uspace);

int detector_open(struct inode *inode, struct file *filp)
{
    struct detector_dev *detector = container_of(inode->i_cdev,struct detector_dev,cdev);
    if(down_interruptible(&detector->sem))
    {
	return -EBUSY;
    }
    filp->private_data=detector;
    return 0;
}


//release并不是在每次close调用后执行，而是在file结构被释放时调用。close的只是个文件描述符
int detector_release(struct inode *inode, struct file *filp)
{
    struct detector_dev *detector=container_of(inode->i_cdev,struct detector_dev,cdev);
    up(&detector->sem);
    return 0;
}

ssize_t detector_read(struct file *filp ,char __user * uspace, size_t count, loff_t *fops)
{
	struct detector_dev *detector = filp->private_data;
	int ret=0;
	
	
	ret=do_detect(detector,count,uspace);
	
	return ret;
	
}

ssize_t detector_write(struct file *filp, const char __user *uspace, size_t count, loff_t *fops)
{
    int retval=0;
//    int ret=0;
    struct detector_dev *detector = filp->private_data;		//获得detector结构体的指针
/*    if(!detector->eff)						//如果上次传送的数据未完全取出，返回-1
    {
	printk("detector: you need read\n");
	return -1;
    }
*/  
    if(count > MAX_SIZE)					//超出缓冲区大小则截断
	count = MAX_SIZE-1;
    
    if(copy_from_user(detector->data,uspace,count))
    {
	retval = -EFAULT;
	goto out;
    }
 
/*    DEBUG("data from user space %s\n",detector->data);
    detector->size=count;	//no use
*/
	detector->data[count]='\0';
    retval = count;
	
	
	//填充辅助结构体
	struct ass_scaddr *ass=detector->ass;
	ass->cmd=detector->data[0];
	ass->mid=0;

    //此处调用处理函数，处理函数将给缓冲区填充数据
/*    ret=do_detect(detector);
    DEBUG("do_detect ret value %d",ret);
    if(ret>0)
    {
	detector->size=ret;
    }
    else
    {
	return ret;
    }
*/  
    *fops=0;		//将偏移置于开头
    
    detector->eff=0;		//将表示位置为没读完
    
    
    
    out:
	return retval;
}

//此函数什么也不做。
loff_t detector_llseek(struct file *filp, loff_t off, int whence)
{
    printk("detector: you should't call lseek\n");
    return 0;
}

struct file_operations detector_fops=
{
    .owner =THIS_MODULE,
    .open=detector_open,
    .read=detector_read,
    .write=detector_write,
    .release=detector_release,
    .llseek=detector_llseek,
};

void detector_exit(void)
{
    int devno = MKDEV(detector_major,detector_minor);
    cdev_del(&detector->cdev);
    if(detector)
    {
		if(detector->data)
			vfree(detector->data);
		if(detector->ass)
			vfree(detector->ass);
		vfree(detector);
    }
    unregister_chrdev_region(devno,NR_MAJOR);
}

int detector_init(void)
{
   dev_t  dev=0;
   int ret;
   
   ret=alloc_chrdev_region(&dev,MY_MAJOR,NR_MAJOR,MY_DEV);	//申请设备号
   detector_major = MAJOR(dev);		//获得主设备号
   printk("detector_major is %d\n",detector_major);
   
   if(ret<0)
   {
       printk("can't get major\n");
       return ret;
   }
   
   detector=vmalloc(sizeof(struct detector_dev));
   DEBUG("detectot address 0x%16lx\n",detector);
   memset(detector,0,sizeof(struct detector_dev));
   if(!detector)
   {
       ret=-ENOMEM;
       goto fail;		//调用清理函数
   }
   
   //初始化设备设备结构体
   sema_init(&detector->sem,1);
   
   //获取用于传输数据的空间
   detector->data=vmalloc(MAX_SIZE);
   DEBUG("detector->data address 0x%16lx\n",detector->data);

   if(detector->data<=0)
   {
	DEBUG("vmalloc MAX_SIZE\n");
	ret=-1;
	goto fail;
   }
   
   //获得辅助空间
   detector->ass=vmalloc(ASS_SIZE);
   DEBUG("detector->ass address 0x%16lx\n",detector->ass);
   if(detector->ass<=0)
   {
	   DEBUG("vmalloc ass memory error\n");
	   ret=-1;
	   goto fail;
   }
   
   
   memset(detector->ass,0,ASS_SIZE);
   detector->size=0;		//初始大小为0
   detector->eff=1;		//初始置于有效
   
   
   int devno = MKDEV(detector_major, detector_minor);	//获得设备号
   cdev_init(&detector->cdev, &detector_fops);		//初始化cdev
   detector->cdev.owner = THIS_MODULE;			
   ret = cdev_add(&detector->cdev,devno,1);	
   if(ret)
	DEBUG("error to add detector\n");
   
   return 0;	//完成

    fail:
    detector_exit();
	return ret;
}

module_init(detector_init);
module_exit(detector_exit);

MODULE_LICENSE("Dual BSD/GPL");
