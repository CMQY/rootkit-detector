#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <asm/uaccess.h>
#include <linux/syscalls.h>
#include "get_sys_addr.h"
#include "detector.h"


unsigned long isfinded=0;
unsigned long ** syscall_table=NULL;


//  return the syscalls table address 
unsigned long ** find_syscall_tables(void)
{
	unsigned long offset = PAGE_OFFSET;
	unsigned long **sct;
	
	while(offset < ULLONG_MAX){
		sct = (unsigned long **)offset;
		
		if(sct[__NR_close]==(unsigned long *) sys_close)
			return sct;
		offset+=sizeof(void *);
	}

	return NULL;
}


#define STRLEN 25
size_t detecte(struct detector_dev * detector,size_t count)
{
	struct ass_scaddr *sca=(struct ass_scaddr *)(detector->ass);
	char temp[STRLEN+1];
	size_t i=sca->mid;
	if(i>CALL32_END)
		return 0;
	int num=count/STRLEN;
	int j;
	memset(detector->data,0,MAX_SIZE);
	
	for(j=0;j<num;j++)
	{
		sprintf(temp,"%d\t%16lx\n",i,(unsigned long)syscall_table[i]);
		strcat(detector->data,temp);
		i++;
		if(i==CALL64_END+1)
			i=CALL32_START;
		if(i>CALL32_END)
			break;
	}
	sca->mid=i;
	
	j=strlen(detector->data);
	return j;
}

size_t do_getsyscalladdr(struct detector_dev * detector,size_t count,char __user *uspace)
{
	int ret=0;
	if(!isfinded)
	{
		syscall_table=find_syscall_tables();
		if(syscall_table==NULL)
		{
			printk("can't find syscall table address...\n");
			return -EFAULT;
		}
		else
		{
			printk("the syscall table address is 0x%16lX\n",syscall_table);
		}
		isfinded=1;
	}

	ret=detecte(detector,count);
	copy_to_user(uspace,detector->data,ret);
	return ret;
}

MODULE_LICENSE("Dual BSD/GPL");
