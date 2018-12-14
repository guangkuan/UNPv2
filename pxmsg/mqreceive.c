#include	"unpipc.h"

int main(int argc, char **argv)
{
	int		c, flags;
	mqd_t	mqd;
	ssize_t	n;
	unsigned int prio;
	void	*buff;
	struct mq_attr	attr;

	flags = O_RDONLY;
	while ( (c = Getopt(argc, argv, "n")) != -1) 
	{
		switch (c) 
		{
		case 'n':
			flags |= O_NONBLOCK;
			break;
		}
	}
	if (optind != argc - 1)
		err_quit("usage: mqreceive [ -n ] <name>");

	mqd = Mq_open(argv[optind], flags);
	//mq_receive的len参数的值不能小于能加到指定队列中的消息的最大大小（该队列mq_attr结构的mq_msgsize成员）
	//这意味着使用Posix消息队列的大多数应用程序必须在打开某个队列后调用mq_getattr确定最大消息大小，然后分配一个或多个那样大小的读缓冲区
	Mq_getattr(mqd, &attr);

	buff = Malloc(attr.mq_msgsize);

	//从mqd指向的文件中读取len长度到缓存buff
	//返回：若成功则位消息中字节数，若出错则为-1
	n = Mq_receive(mqd, buff, attr.mq_msgsize, &prio);
	printf("read %ld bytes, priority = %u\n", (long) n, prio);

	exit(0);
}
