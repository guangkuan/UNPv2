#include	"unpipc.h"

volatile sig_atomic_t	mqflag;		/* set nonzero by signal handler */
static void	sig_usr1(int);

int main(int argc, char **argv)
{
	mqd_t	mqd;
	void	*buff;
	ssize_t	n;
	sigset_t	zeromask, newmask, oldmask;
	struct mq_attr	attr;
	struct sigevent	sigev;

	if (argc != 2)
		err_quit("usage: mqnotifysig2 <name>");

		/* 4open queue, get attributes, allocate read buffer */
	mqd = Mq_open(argv[1], O_RDONLY);
	Mq_getattr(mqd, &attr);
	buff = Malloc(attr.mq_msgsize);

	//将信号集初始化为空
	Sigemptyset(&zeromask);		/* no signals blocked */
	Sigemptyset(&newmask);
	Sigemptyset(&oldmask);
	//把信号SIGUSR1添加到信号集newmask中，成功时返回0，失败时返回-1
	Sigaddset(&newmask, SIGUSR1);

		/* 4establish signal handler, enable notification */
	Signal(SIGUSR1, sig_usr1);
	sigev.sigev_notify = SIGEV_SIGNAL;
	sigev.sigev_signo = SIGUSR1;
	Mq_notify(mqd, &sigev);

	for ( ; ; ) 
	{
		//根据参数指定的方法修改进程的信号屏蔽字。新的信号屏蔽字由参数newmask（非空）指定，而原先的信号屏蔽字将保存在oldmask（非空）中。
		//如果newmask为空，则第一个参数没有意义，但此时调用该函数，如果oldmask不为空，则把当前信号屏蔽字保存到oldmask中。
		//SIG_BLOCK 把参数newmask中的信号添加到信号屏蔽字中
		Sigprocmask(SIG_BLOCK, &newmask, &oldmask);	/* block SIGUSR1 */

		while (mqflag == 0)
		{
			printf("suspend...\n");
			//与pause()功能比较像
			//该函数通过将进程的屏蔽字替换为由参数zeromask给出的信号集，然后挂起进程的执行。
			//注意操作的先后顺序，是先替换再挂起程序的执行。程序将在信号处理函数执行完毕后继续执行。
			//如果接收到信号终止了程序，sigsuspend()就不会返回，如果接收到的信号没有终止程序，sigsuspend()就返回-1，并将errno设置为EINTR
			sigsuspend(&zeromask);
		}
			
		mqflag = 0;		/* reset flag */

		Mq_notify(mqd, &sigev);			/* reregister first */
		n = Mq_receive(mqd, buff, attr.mq_msgsize, NULL);
		printf("read %ld bytes\n", (long) n);
		//SIG_UNBLOCK 从信号屏蔽字中删除参数newmask中的信号
		Sigprocmask(SIG_UNBLOCK, &newmask, NULL);	/* unblock SIGUSR1 */
	}
	exit(0);
}

static void sig_usr1(int signo)
{
	mqflag = 1;
	printf("signal 1\n");
	return;
}
