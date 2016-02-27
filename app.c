#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

void printusg(int argc,char *argv[])
{
	printf("usage: %s <-a|-b>\n",argv[0]);
	exit(-1);
}

int main(int argc, char *argv[])
{
	if(argc!=2)
		printusg(argc,argv);
	
	int fd;
	size_t count=4096;
	int re=0;
	char *buf=NULL;
	fd=open("dete",O_RDWR);
	if(fd<0)
		{
			perror("open");
			return -1;
		}

	
	switch(argv[1][1])
	{
	case 'a':	//syscall address
		buf=malloc(count);
		if(!buf)
		{
			perror("malloc");
			return -1;
		}
		re=write(fd,"a",1);
		if(re<0)
		{
			perror("write");
			return -1;
		}
		while((re=read(fd,buf,count-1))>0)
		{
			buf[re]='\0';
			printf("%s",buf);
		}
		break;
	case 'b':	//proc info
		buf=malloc(count);
		if(!buf)
		{
			perror("malloc");
			return -1;
		}

		re=write(fd,"b",1);
		if(re<0)
		{
			perror("write");
			return -1;
		}
	
		while(1)
		{
			re=read(fd,buf,count);
			if(re<0)
			{
				count=*(size_t *)buf;
				free(buf);
				buf=malloc(count);
				if(!buf)
				{
					perror("malloc");
					return -1;
				}
				continue;
			}
			printf("%s",buf);
			break;
		}
		break;
	default:
		printusg(argc,argv);
	}



	close(fd);
	return 0;
}
