ifneq ($(KERNELRELEASE),)
	obj-m :=dete.o
	dete-objs :=detector.o reader.o get_sys_addr.o get_procinfo.o
else
	KERNELDIR?=/lib/modules/$(shell uname -r)/build
	PWD :=$(shell pwd)
default:
		$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
		gcc app.c -o app
endif

clean:
	rm -rf *.mod.*
	rm -rf .*.cmd
	rm -rf *.o


