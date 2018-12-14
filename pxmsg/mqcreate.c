#include	"unpipc.h"

struct mq_attr	attr;	/* mq_maxmsg and mq_msgsize both init to 0 */

int
main(int argc, char **argv)
{
	int		c, flags;
	mqd_t	mqd;

	flags = O_RDWR | O_CREAT;
	//getopt()用来分析命令行参数
	while ( (c = Getopt(argc, argv, "em:z:")) != -1) 
	{
		switch (c) 
		{
			case 'e':
				flags |= O_EXCL;
				break;

			case 'm':
				attr.mq_maxmsg = atol(optarg);
				break;

			case 'z':
				attr.mq_msgsize = atol(optarg);
				break;
		}
	}
	if (optind != argc - 1)
		err_quit("usage: mqcreate [ -e ] [ -m maxmsg -z msgsize ] <name>");

	if ((attr.mq_maxmsg != 0 && attr.mq_msgsize == 0) || (attr.mq_maxmsg == 0 && attr.mq_msgsize != 0))
		err_quit("must specify both -m maxmsg and -z msgsize");

	//第四个参数attr包括每个队列的最大消息数，和每个消息的最大字节数等四个信息
	//创建队列时两者都得指定
	mqd = Mq_open(argv[optind], flags, FILE_MODE, (attr.mq_maxmsg != 0) ? &attr : NULL);

	Mq_close(mqd);
	exit(0);
}
