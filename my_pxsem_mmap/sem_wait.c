/* include sem_wait */
#include	"unpipc.h"
#include	"semaphore.h"

int mysem_wait(mysem_t *sem)
{
	int		n;

	if (sem->sem_magic != SEM_MAGIC) 
	{
		errno = EINVAL;
		return(-1);
	}

	if ( (n = pthread_mutex_lock(&sem->sem_mutex)) != 0) 
	{
		errno = n;
		return(-1);
	}
	//如果信号量的值为0，则让调用线程睡眠在一个phtread_cond_wait调用中
	while (sem->sem_count == 0)
		pthread_cond_wait(&sem->sem_cond, &sem->sem_mutex);
	//等待另外某个线程为该信号量的条件变量调用pthread_cond_signal，当信号量大于0时，将信号量减1
	sem->sem_count--;
	pthread_mutex_unlock(&sem->sem_mutex);
	return(0);
}
/* end sem_wait */

void Mysem_wait(mysem_t *sem)
{
	if (mysem_wait(sem) == -1)
		err_sys("mysem_wait error");
}
