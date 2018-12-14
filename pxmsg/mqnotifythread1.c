#include	"unpipc.h"

mqd_t	mqd;
struct mq_attr	attr;
struct sigevent	sigev;

static void	notify_thread(union sigval);		/* our thread function */

int main(int argc, char **argv)
{
	if (argc != 2)
		err_quit("usage: mqnotifythread1 <name>");

	mqd = Mq_open(argv[1], O_RDONLY | O_NONBLOCK);
	Mq_getattr(mqd, &attr);

	//异步事件通知的另一种方式是把sigev_notify设置成SIGEV_THREAD，这会创建一个新的线程。
	//该线程调用由sigev_notify_function指定的函数，所用参数由sigev_value指定。新线程的线程属性由sigev_notify_attributes指定，要是默认属性合适的话，它可以是一个空指针。
	sigev.sigev_notify = SIGEV_THREAD;
	sigev.sigev_value.sival_ptr = NULL;
	sigev.sigev_notify_function = notify_thread;
	sigev.sigev_notify_attributes = NULL;
	Mq_notify(mqd, &sigev);

	for ( ; ; )
		pause();		/* each new thread does everything */

	exit(0);
}

static void notify_thread(union sigval arg)
{
	ssize_t	n;
	void	*buff;

	printf("notify_thread started\n");
	buff = Malloc(attr.mq_msgsize);
	Mq_notify(mqd, &sigev);			/* reregister */

	while ( (n = mq_receive(mqd, buff, attr.mq_msgsize, NULL)) >= 0) 
	{
		printf("read %ld bytes\n", (long) n);
	}
	if (errno != EAGAIN)
		err_sys("mq_receive error");

	free(buff);
	pthread_exit(NULL);
}
