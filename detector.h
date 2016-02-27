#include <linux/cdev.h> 
#include <linux/semaphore.h>

#define MY_DEV "detector"
#define MY_MAJOR 1
#define NR_MAJOR 1

#define DATA_PAGE_ORDER 0	//内核内存页order
#define PAGES (1<<DATA_PAGE_ORDER)
#define MAX_SIZE (PAGES*PAGE_SIZE)

#define MAX_ARG_SIZE 100	
#define ASS_SIZE	20		//辅助定位空间大小

#ifdef	DEBUG_INFO

#define DEBUG(x,arg...) printk(x,##arg);

#else

#define DEBUG(x,arg...)

#endif

struct detector_dev{
    char *data;			//数据区
    unsigned long size;		//有效数据长度
    int eff;			//数据是否读取完成
	char *ass;			//辅助定位
    struct semaphore sem;	//数据区信号量
    
    struct cdev cdev;		//字符设备结构
};


struct ass_scaddr{
	char cmd;
	size_t mid;
};
