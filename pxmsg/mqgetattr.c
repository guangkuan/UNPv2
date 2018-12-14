#include	"unpipc.h"

int main(int argc, char **argv)
{
	mqd_t	mqd;
	struct mq_attr	attr;

	if (argc != 2)
		err_quit("usage: mqgetattr <name>");

	mqd = Mq_open(argv[1], O_RDONLY);

	//指向某个mq_attr结构的指针可作为mq_open的第四个参数传递，从而允许我们在创建一个新队列时，给它指定mq_maxmsg和mq_msgsize属性。mq_open忽略该结构的另外两个成员
	//队列中的当前消息数则只能获取而不能设置
	Mq_getattr(mqd, &attr);
	printf("max #msgs = %ld, max #bytes/msg = %ld, #currently on queue = %ld\n", attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);

	Mq_close(mqd);
	exit(0);
}
