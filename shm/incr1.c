#include	"unpipc.h"

#define	SEM_NAME	"mysem"

int		count = 0;

int
main(int argc, char **argv)
{
	int		i, nloop;
	sem_t	*mutex;

	if (argc != 2)
		err_quit("usage: incr1 <#loops>");
	nloop = atoi(argv[1]);

		/* 4create, initialize, and unlink semaphore */
	mutex = Sem_open(Px_ipc_name(SEM_NAME), O_CREAT | O_EXCL, FILE_MODE, 1);
	Sem_unlink(Px_ipc_name(SEM_NAME));

	//将标准输出设置为非缓冲模式，为防止两个进程的输出不恰当的交叉
	setbuf(stdout, NULL);	/* stdout is unbuffered */
	//两个进程都有各自的全局变量count副本，每个进程都从该变量为0的初始值开始，而且每次加1的对象是各自的变量的副本
	if (Fork() == 0) 
	{		/* child */
		for (i = 0; i < nloop; i++) 
		{
			Sem_wait(mutex);
			printf("child: %d\n", count++);
			Sem_post(mutex);
		}
		exit(0);
	}

		/* 4parent */
	for (i = 0; i < nloop; i++) 
	{
		Sem_wait(mutex);
		printf("parent: %d\n", count++);
		Sem_post(mutex);
	}
	exit(0);
}
