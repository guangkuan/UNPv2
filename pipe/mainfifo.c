#include	"unpipc.h"

#define	FIFO1	"/tmp/fifo.10"
#define	FIFO2	"/tmp/fifo.20"

void	client(int, int), server(int, int);

int
main(int argc, char **argv)
{
	int		readfd, writefd;
	pid_t	childpid;

		/* 4create two FIFOs; OK if they already exist */
	//与pipe相似FIFO类似于管道，它是一个单向（半双工）数据流。
	//不同于管道的是，每个FIFO都有一个路径名与之关联，从而允许无亲缘关系的进程访问同一个FIFO
	//创建并打开一个FIFO则需在调用mkfifo后再调用open
	if ((mkfifo(FIFO1, FILE_MODE) < 0) && (errno != EEXIST))
		err_sys("can't create %s", FIFO1);
	if ((mkfifo(FIFO2, FILE_MODE) < 0) && (errno != EEXIST)) 
	{
		unlink(FIFO1);
		err_sys("can't create %s", FIFO2);
	}

	//客户与服务器有亲缘关系
	if ( (childpid = Fork()) == 0) 
	{		/* child */
		readfd = Open(FIFO1, O_RDONLY, 0);
		writefd = Open(FIFO2, O_WRONLY, 0);

		server(readfd, writefd);
		exit(0);
	}
		/* 4parent */
	//如果当前尚没有任何进程打开某个FIFO来写，那么打开该FIFO来读的进程将阻塞
	//对换父进程中两个open的调用顺序后，父子进程将都打开同一个FIFO来读，然而当时并没有任何进程打开该文件来写，于是父子进程同时阻塞，这种现象称为死锁(deadlock)
	writefd = Open(FIFO1, O_WRONLY, 0);
	readfd = Open(FIFO2, O_RDONLY, 0);

	client(readfd, writefd);

	Waitpid(childpid, NULL, 0);		/* wait for child to terminate */

	Close(readfd);
	Close(writefd);

	//pipe在所有进程最终都关闭它之后自动消失
	//FIFO的名字则只有通过调用unlink才从文件系统中删除
	Unlink(FIFO1);
	Unlink(FIFO2);
	exit(0);
}
