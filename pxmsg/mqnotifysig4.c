#include	"unpipc.h"

int main(int argc, char **argv)
{
	int		signo;
	mqd_t	mqd;
	void	*buff;
	ssize_t	n;
	sigset_t	newmask;
	struct mq_attr	attr;
	struct sigevent	sigev;

	if (argc != 2)
		err_quit("usage: mqnotifysig4 <name>");

		/* 4open queue, get attributes, allocate read buffer */
	mqd = Mq_open(argv[1], O_RDONLY | O_NONBLOCK);
	Mq_getattr(mqd, &attr);
	buff = Malloc(attr.mq_msgsize);

	Sigemptyset(&newmask);
	Sigaddset(&newmask, SIGUSR1);
	//阻塞信号集newmask
	Sigprocmask(SIG_BLOCK, &newmask, NULL);		/* block SIGUSR1 */

		/* 4establish signal handler, enable notification */
	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = SIGUSR1;
	Mq_notify(mqd, &sigev);

	for ( ; ; ) 
	{
		//sigwait然后一直阻塞到这些信号中有一个或多个待处理，这时它返回其中一个信号。这个过程成为“同步地等待一个异步事件”
		Sigwait(&newmask, &signo);
		if (signo == SIGUSR1) 
		{
			Mq_notify(mqd, &sigev);			/* reregister first */
			while ( (n = mq_receive(mqd, buff, attr.mq_msgsize, NULL)) >= 0) 
			{
				printf("read %ld bytes\n", (long) n);
			}
			if (errno != EAGAIN)
				err_sys("mq_receive error");
		}
	}
	exit(0);
}
