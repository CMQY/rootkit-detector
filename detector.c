#include <linux/unistd.h>
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/types.h>

#define DEBUG_INFO
#include "detector.h" 
#define TEMPBUFSIZE 256
extern size_t do_getsyscalladdr(struct detector_dev *,size_t,char __user * uspace);
extern size_t do_getprocinfo(struct detector_dev *,size_t ,char __user *uspace);

int do_detect(struct detector_dev *detector,size_t count,char __user * uspace)
{
	int ret=0;
	struct ass_scaddr *sca=(struct ass_scaddr *)(detector->ass);
	switch(sca->cmd)
	{
	case 'a' :
		if(count>MAX_SIZE)
			count=MAX_SIZE;
		ret=do_getsyscalladdr(detector,count,uspace);
		break;
	case 'b' :
		DEBUG("excuting\n");
		ret=do_getprocinfo(detector,count,uspace);
		break;
	}
	
	return ret;
}
    
    
    
