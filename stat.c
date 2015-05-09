#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include "job.h"

// #define DEBUG	// 调试任务5@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ 

/* 
 * 命令语法格式
 *     stat
 */
void usage()
{
	printf("Usage: stat\n");		
}

int main(int argc,char *argv[])
{
	struct jobcmd statcmd;
	int fd;
	/* 命令参数出错，提示语法信息 */
	if(argc!=1)
	{
		usage();
		return 1;
	}

	statcmd.type=STAT;
	statcmd.defpri=0;
	statcmd.owner=getuid();
	statcmd.argnum=0;
/* 调试任务5@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */
#ifdef DEBUG
printf("statcmd cmdtype\t%d\n"
	"statcmd owner\t%d\n",
	statcmd.type,statcmd.owner);
#endif
	/* 打开fifo文件 */
	if((fd=open("/tmp/server",O_WRONLY))<0)
		error_sys("stat open fifo failed");

	if(write(fd,&statcmd,DATALEN)<0)
		error_sys("stat write failed");

	close(fd);
	return 0;
}
