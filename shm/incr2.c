#include	"unpipc.h"

#define	SEM_NAME	"/mysem"

int
main(int argc, char **argv)
{
	int		fd, i, nloop, zero = 0;
	int		*ptr;
	sem_t	*mutex;

	if (argc != 3)
		err_quit("usage: incr2 <pathname> <#loops>");
	nloop = atoi(argv[2]);

		/* 4open file, initialize to 0, map into memory */
	fd = Open(argv[1], O_RDWR | O_CREAT, FILE_MODE);
	Write(fd, &zero, sizeof(int));
	//mmap函数的返回值都是描述符fd所映射到内存区的起始地址
	ptr = Mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	Close(fd);

		/* 4create, initialize, and unlink semaphore */
	mutex = Sem_open(SEM_NAME, O_CREAT | O_EXCL, FILE_MODE, 1);
	Sem_unlink(SEM_NAME);

	setbuf(stdout, NULL);	/* stdout is unbuffered */
	/*fork对内存映射文件进行特殊处理，也就是符进程在调用fork之前创建的内存映射关系由子进程共享
	/我们在打开文件后以MAP_SHARED标志调用mmap的操作实际上提供了一个由父子进程共享的内存区
	/而且，对它所作的任何改动还会反映到真正的文件中
	/父子进程都各自有属于自己的指针ptr的副本，但是每个副本都指向共享内存区中的同一个整数
	*/
	if (Fork() == 0) 
	{		/* child */
		for (i = 0; i < nloop; i++) 
		{
			Sem_wait(mutex);
			printf("child: %d\n", (*ptr)++);
			Sem_post(mutex);
		}
		exit(0);
	}

		/* 4parent */
	for (i = 0; i < nloop; i++) 
	{
		Sem_wait(mutex);
		printf("parent: %d\n", (*ptr)++);
		Sem_post(mutex);
	}
	exit(0);
}
