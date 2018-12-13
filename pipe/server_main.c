#include	"fifo.h"

void	server(int, int);

int main(int argc, char **argv)
{
	int		readfd, writefd;

		/* 4create two FIFOs; OK if they already exist */
	if ((mkfifo(FIFO1, FILE_MODE) < 0) && (errno != EEXIST))
		err_sys("can't create %s", FIFO1);
	if ((mkfifo(FIFO2, FILE_MODE) < 0) && (errno != EEXIST)) 
	{
		unlink(FIFO1);
		err_sys("can't create %s", FIFO2);
	}

	//对管道或FIFO的wirte总是往末尾添加数据，对它们的read则总是从开头返回数据。
	//如果对管道或FIFO调用lseek，那就返回ESPIPE错误
	readfd = Open(FIFO1, O_RDONLY, 0);
	writefd = Open(FIFO2, O_WRONLY, 0);

	server(readfd, writefd);
	exit(0);
}
