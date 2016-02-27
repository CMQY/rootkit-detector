#include <linux/unistd.h>
#include <linux/sched.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include "detector.h"

#define __PROCINFO_SIZE (TASK_COMM_LEN+4+4+4+4+1)
#define PROCINFO_SIZE __PROCINFO_SIZE

size_t do_getprocinfo(struct detector_dev *detector,size_t count,char __user *uspace)
{
	struct task_struct * p=NULL;
	int first=0;
	int second=0;
	int str_len=0;
	int sum=0;
	size_t total=0;
	char * usp_add=uspace;
	char tempbuf[PROCINFO_SIZE];
	for_each_process(p)
		first++;
	total=first*PROCINFO_SIZE+1;

	if(count<total)
	{
		printk("excuting here\n");
		copy_to_user(uspace,&total,sizeof(total));
		return -1;
	}
	
	
	for_each_process(p)
	{
		second++;
		if(second>first)
			break;
		sprintf(&tempbuf,"%lu\t%lu\t%s\t%lu\n",p->pid,p->tgid,p->comm,p->parent->pid);
		str_len=strlen(&tempbuf);
		copy_to_user(usp_add,&tempbuf,str_len);
		sum+=str_len;
		usp_add+=str_len;
	}
	copy_to_user(usp_add,"\0",1);
	return sum+1;
}	
