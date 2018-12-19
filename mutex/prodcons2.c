/* include main */
#include	"unpipc.h"

#define	MAXNITEMS 		1000000
#define	MAXNTHREADS			100

int		nitems;			/* read-only by producer and consumer */
/*Posix互斥锁被声明为具有pthread_mutex_t数据类型的变量
/如果互斥锁变量是静态分配的，那么我们可以把它初始化成常值PTHREAD_MUTEX_ININIALIZER
/如果互斥锁是动态分配的（例如通过调用malloc），或者分配在共享内存区中，那么我们必须在运行之时通过调用pthread_mutex_init函数来初始化它
*/
struct {
  pthread_mutex_t	mutex;
  int	buff[MAXNITEMS];
  //nput是buff数组中下一次存放的元素下标
  int	nput;
  //nval是下一次存放的值(0、1、2等)
  int	nval;
} shared = { PTHREAD_MUTEX_INITIALIZER };//初始化其中用于生产者线程间同步的互斥锁

void	*produce(void *), *consume(void *);

/*互斥锁指代相互排斥(mutual exclusion)，它是最基本的同步形式。互斥锁用于保护临界区(critical region)
/保证任何时刻只有一个线程在执行其中的代码（假设互斥锁由多个线程共享）
/或者任何时刻只有一个进程在执行其中的代码（假设互斥锁由多个进程共享）
/尽管我们说互斥锁保护的是临界区，实际上保护的是在临界区中被操纵的数据(data)。
/也就是说，互斥锁通常用于保护由多个线程或多个进程共享的共享数据(shared data)
*/
int main(int argc, char **argv)
{
	int			i, nthreads, count[MAXNTHREADS];
	pthread_t	tid_produce[MAXNTHREADS], tid_consume;

	if (argc != 3)
		err_quit("usage: prodcons2 <#items> <#threads>");
	//命令行参数1指定生产者存放的条目数，命令行参数2指定待创建生产者线程的数目
	nitems = min(atoi(argv[1]), MAXNITEMS);
	nthreads = min(atoi(argv[2]), MAXNTHREADS);

	//告诉线程系统我们希望并发运行多少线程
	Set_concurrency(nthreads);
		/* 4start all the producer threads */
	for (i = 0; i < nthreads; i++) 
	{
		count[i] = 0;
		//对应参数(指向线程标识符的指针，线程属性，线程运行函数的地址，运行函数的参数)
		//在tid_produce数组中保存每个线程的线程ID
		Pthread_create(&tid_produce[i], NULL, produce, &count[i]);
	}

		/* 4wait for all the producer threads */
	for (i = 0; i < nthreads; i++) 
	{
		//等待所有的生产者线程终止
		Pthread_join(tid_produce[i], NULL);
		printf("count[%d] = %d\n", i, count[i]);	
	}

		/* 4start, then wait for the consumer thread */
	//在生产者都终止后，才启动单个的消费者线程。这是我们暂时避免生产者和消费者之间同步问题的办法
	Pthread_create(&tid_consume, NULL, consume, NULL);
	Pthread_join(tid_consume, NULL);

	exit(0);
}
/* end main */

/* include producer */
void* produce(void *arg)
{
	for ( ; ; ) 
	{
		//如果尝试给一个已由另外某个线程锁住的互斥锁上锁，那么pthread_mutex_lock将阻塞到该互斥锁解锁为止
		Pthread_mutex_lock(&shared.mutex);
		if (shared.nput >= nitems) 
		{
			Pthread_mutex_unlock(&shared.mutex);
			return(NULL);		/* array is full, we're done */
		}
		shared.buff[shared.nput] = shared.nval;
		shared.nput++;
		shared.nval++;
		Pthread_mutex_unlock(&shared.mutex);
		//每个线程在每次往缓冲区中存放一个条目时给这个计数器加1
		//每个线程有各自的计数器(main函数中的count数组)
		*((int *) arg) += 1;
	}
}

void* consume(void *arg)
{
	int		i;

	for (i = 0; i < nitems; i++) 
	{
		if (shared.buff[i] != i)
			printf("buff[%d] = %d\n", i, shared.buff[i]);
	}
	return(NULL);
}
/* end producer */
