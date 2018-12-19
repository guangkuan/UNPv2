#include	"unpipc.h"

#define	MAXNITEMS 		1000000
#define	MAXNTHREADS			100

int		nitems;			/* read-only by producer and consumer */
struct {
  pthread_mutex_t	mutex;
  int	buff[MAXNITEMS];
  int	nput;
  int	nval;
} shared = { PTHREAD_MUTEX_INITIALIZER };

void	*produce(void *), *consume(void *);

/* include main */
int main(int argc, char **argv)
{
	int			i, nthreads, count[MAXNTHREADS];
	pthread_t	tid_produce[MAXNTHREADS], tid_consume;

	if (argc != 3)
		err_quit("usage: prodcons3 <#items> <#threads>");
	nitems = min(atoi(argv[1]), MAXNITEMS);
	nthreads = min(atoi(argv[2]), MAXNTHREADS);

		/* 4create all producers and one consumer */
	Set_concurrency(nthreads + 1);
	for (i = 0; i < nthreads; i++) 
	{
		count[i] = 0;
		Pthread_create(&tid_produce[i], NULL, produce, &count[i]);
	}
	//所有生产者启动后立即启动消费者
	//在生产者线程产生数据的同时，消费者线程就能处理它
	Pthread_create(&tid_consume, NULL, consume, NULL);

		/* 4wait for all producers and the consumer */
	for (i = 0; i < nthreads; i++) 
	{
		Pthread_join(tid_produce[i], NULL);
		printf("count[%d] = %d\n", i, count[i]);	
	}
	Pthread_join(tid_consume, NULL);

	exit(0);
}
/* end main */

void* produce(void *arg)
{
	for ( ; ; ) {
		//任意时刻只有一个线程能够锁住一个给定的互斥锁，于是保证任何时刻只有一个线程在执行其临界区中的指令
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
		*((int *) arg) += 1;
	}
}

/* include consume */
void consume_wait(int i)
{
	//一次次地循环，每次给互斥锁解锁又上锁
	//轮转(spinning)或轮询(polling)
	for ( ; ; ) 
	{
		Pthread_mutex_lock(&shared.mutex);
		if (i < shared.nput) 
		{
			//printf("waiting ......\n");
			Pthread_mutex_unlock(&shared.mutex);
			return;			/* an item is ready */
		}
		Pthread_mutex_unlock(&shared.mutex);
	}
}

void* consume(void *arg)
{
	int		i;

	for (i = 0; i < nitems; i++) 
	{
		consume_wait(i);
		if (shared.buff[i] != i)
			printf("buff[%d] = %d\n", i, shared.buff[i]);
	}
	return(NULL);
}
/* end consume */
