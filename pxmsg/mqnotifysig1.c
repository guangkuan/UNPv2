#include	"unpipc.h"

mqd_t	mqd;
void	*buff;
struct mq_attr	attr;
struct sigevent	sigev;

static void	sig_usr1(int);

//任意时刻只有一个进程可以被注册为接收某个给定队列的通知
int main(int argc, char **argv)
{
	if (argc != 2)
		err_quit("usage: mqnotifysig1 <name>");

		/* 4open queue, get attributes, allocate read buffer */
	mqd = Mq_open(argv[1], O_RDONLY);
	Mq_getattr(mqd, &attr);
	buff = Malloc(attr.mq_msgsize);

		/* 4establish signal handler, enable notification */
	Signal(SIGUSR1, sig_usr1);
	//SIGEV_SIGNAL常值的意思是当所指定队列由空变为非空时，我们希望有一个信号会产生
	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = SIGUSR1;
	//如果sigevent类型参数非空，那么当前进程希望在有一个消息到达指定的先前为空的队列时得到通知。我们说“该进程被注册为接收该队列的通知”
	Mq_notify(mqd, &sigev);

	for ( ; ; )
		//挂起知道有信号传递过来
		pause();		/* signal handler does everything */
	exit(0);
}

static void sig_usr1(int signo)
{
	ssize_t	n;

	//当该通知被发送给它的注册进程时，其注册即被撤销。该进程必须再次调用mq_notify以重新注册
	Mq_notify(mqd, &sigev);			/* reregister first */
	n = Mq_receive(mqd, buff, attr.mq_msgsize, NULL);
	printf("SIGUSR1 received, read %ld bytes\n", (long) n);
	return;
}
