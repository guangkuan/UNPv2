#include	"unpipc.h"

int main(int argc, char **argv)
{
	int		c, flags;
	mqd_t	mqd;

	flags = O_RDWR | O_CREAT;
	while ( (c = Getopt(argc, argv, "e")) != -1) 
	{
		switch (c) 
		{
		case 'e':
			flags |= O_EXCL;
			break;
		}
	}
	if (optind != argc - 1)
		err_quit("usage: mqcreate [ -e ] <name>");
	//man -a mq_overview 查看建立文件需要挂载
	//mq_open的返回值称为消息队列描述符，但它不必是（而且很可能不是）像文件描述符或套接字描述符的短整数
	//第四个参数attr用于给新队列指定某些属性，如果它为空指针，那就使用默认属性
	mqd = Mq_open(argv[optind], flags, FILE_MODE, NULL);

	Mq_close(mqd);
	exit(0);
}
