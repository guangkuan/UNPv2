#include	"unpipc.h"

static void	sig_rt(int, siginfo_t *, void *);

int main(int argc, char **argv)
{
	int		i, j;
	pid_t	pid;
	sigset_t	newset;
	union sigval	val;

	//输出最小和最大实时信号值，以查看系统实现支持多少种实时信号
	printf("SIGRTMIN = %d, SIGRTMAX = %d\n", (int) SIGRTMIN, (int) SIGRTMAX);

	if ( (pid = Fork()) == 0) 
	{
			/* 4child: block three realtime signals */
		Sigemptyset(&newset);

		//实时行为(realtime behavior)隐含着如下特征
		/*信号是排队的
		/ 当有多个SIGRTMIN到SIGRTMAX范围内的解阻塞信号排队时，值较小的信号先于值较大的信号递交
		/ 当某个非实时信号递交时，传递给它的信号处理程序的唯一参数是该信号的值
		/ 一些新函数定义成使用实时信号工作。例如，sigqueue函数用于代替kill函数向某个进程发送一个信号*/
		//用sigprocmask阻塞我们将使用的三种实时信号：SIGRTMAX、SIGRTMAX-1、SIGRTMAX-2
		Sigaddset(&newset, SIGRTMAX);
		Sigaddset(&newset, SIGRTMAX - 1);
		Sigaddset(&newset, SIGRTMAX - 2);
		Sigprocmask(SIG_BLOCK, &newset, NULL);

			/* 4establish signal handler with SA_SIGINFO set */
		Signal_rt(SIGRTMAX, sig_rt);
		Signal_rt(SIGRTMAX - 1, sig_rt);
		Signal_rt(SIGRTMAX - 2, sig_rt);

		//等待6秒以允许父进程产生预定的9个信号
		sleep(6);		/* let parent send all the signals */

		//调用sigprocmask解阻塞那三种实时信号，该操作允许所有排队的信号都被递交
		Sigprocmask(SIG_UNBLOCK, &newset, NULL);	/* unblock */
		//等待信号处理程序printf9次
		sleep(3);		/* let all queued signals be delivered */
		exit(0);
	}

		/* 4parent sends nine signals to child */
	//等待子进程阻塞所有信号
	sleep(3);		/* let child block all signals */
	for (i = SIGRTMAX; i >= SIGRTMAX - 2; i--) 
	{
		for (j = 0; j <= 2; j++) 
		{
			val.sival_int = j;
			//If  the receiving process has installed a handler for this signal using the SA_SIGINFO flag to sigaction(2), 
			//then it can obtain this data via the si_value field of the siginfo_t structure passed as the second argument to the handler.  
			//Furthermore, the si_code field of that structure will be set to SI_QUEUE.
			Sigqueue(pid, i, val);
			printf("sent signal %d, val = %d\n", i, j);
		}
	}
	exit(0);
}
//当sigaction(signo, &act, &oact)函数中结构体sigaction的sa_flags被设置为SA_SIGINFO时，结构体使用sa_sigaction(三个参数，siginfo_t是第二个参数的类型)替代sa_handler(一个参数)，
static void sig_rt(int signo, siginfo_t *info, void *context)
{
	printf("received signal #%d, code = %d, ival = %d\n", signo, info->si_code, info->si_value.sival_int);
}
