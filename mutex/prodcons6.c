/* include globals */
#include	"unpipc.h"

#define	MAXNITEMS 		1000000
#define	MAXNTHREADS			100

		/* globals shared by threads */
int		nitems;				/* read-only by producer and consumer */
int		buff[MAXNITEMS];
struct {
  pthread_mutex_t	mutex;
  int				nput;	/* next index to store */
  int				nval;	/* next value to store */
} put = { PTHREAD_MUTEX_INITIALIZER };

//条件变量用于等待，条件变量是类型为pthread_cond_t的变量
struct {
  pthread_mutex_t	mutex;
  pthread_cond_t	cond;
  int				nready;	/* number ready for consumer */
} nready = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER };
/* end globals */

void	*produce(void *), *consume(void *);

/* include main */
int main(int argc, char **argv)
{
	int			i, nthreads, count[MAXNTHREADS];
	pthread_t	tid_produce[MAXNTHREADS], tid_consume;

	if (argc != 3)
		err_quit("usage: prodcons6 <#items> <#threads>");
	nitems = min(atoi(argv[1]), MAXNITEMS);
	nthreads = min(atoi(argv[2]), MAXNTHREADS);

	Set_concurrency(nthreads + 1);
		/* 4create all producers and one consumer */
	for (i = 0; i < nthreads; i++) 
	{
		count[i] = 0;
		Pthread_create(&tid_produce[i], NULL, produce, &count[i]);
	}
	Pthread_create(&tid_consume, NULL, consume, NULL);

		/* wait for all producers and the consumer */
	for (i = 0; i < nthreads; i++) 
	{
		Pthread_join(tid_produce[i], NULL);
		printf("count[%d] = %d\n", i, count[i]);	
	}
	Pthread_join(tid_consume, NULL);

	exit(0);
}
/* end main */

/* include prodcons */
void* produce(void *arg)
{
	for ( ; ; ) 
	{
		Pthread_mutex_lock(&put.mutex);
		if (put.nput >= nitems) 
		{
			Pthread_mutex_unlock(&put.mutex);
			return(NULL);		/* array is full, we're done */
		}
		buff[put.nput] = put.nval;
		put.nput++;
		put.nval++;
		Pthread_mutex_unlock(&put.mutex);

		Pthread_mutex_lock(&nready.mutex);
		if (nready.nready == 0)
		{
			printf("触发...\n");
			//pthread_cond_signal函数的作用是发送一个信号给另外一个正在处于阻塞等待状态的线程,使其脱离阻塞状态,继续执行。
			//如果没有线程处在阻塞等待状态,pthread_cond_signal也会成功返回。
			//因为该区域的互斥锁与函数将给它发送信号的条件变量关联，pthread_cond_wait部分也需要互斥锁
			//e.g. 该条件变量被发送信号后，系统立即调度等待在其上的线程，该线程开始运行，但立即停止，因为它没能获取相应的互斥锁
			Pthread_cond_signal(&nready.cond);
		}
		
		//统计准备好由消费者处理的条目数的计数器nready.nready
		nready.nready++;
		printf("producter %d\n", nready.nready);
		Pthread_mutex_unlock(&nready.mutex);

		*((int *) arg) += 1;
	}
}

void* consume(void *arg)
{
	int		i;

	for (i = 0; i < nitems; i++) 
	{
		//计数器是在所有的生产者和消费者之间共享的，那么只有锁住与之关联的互斥锁(nready.mutex)时才能测试它的值
		Pthread_mutex_lock(&nready.mutex);
		while (nready.nready == 0)
		{
			printf("等待...\n");
			//给互斥锁nready.mutex解锁
			//把调用线程投入睡眠，知道另外某个线程就本条件变量调用pthread_cond_signal
			Pthread_cond_wait(&nready.cond, &nready.mutex);	
		}
		//pthread_cond_wait在返回前重新给互斥锁nready.mutex上锁
		//在我们上锁后才能操作计数器，因为计数器是共享数据
		nready.nready--;
		printf("consumer %d\n", nready.nready);
		Pthread_mutex_unlock(&nready.mutex);

		if (buff[i] != i)
			printf("buff[%d] = %d\n", i, buff[i]);
	}
	return(NULL);
}
/* end prodcons */
