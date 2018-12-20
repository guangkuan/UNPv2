/* include wrlock */
#include	"unpipc.h"
#include	"pthread_rwlock.h"

int my_pthread_rwlock_wrlock(my_pthread_rwlock_t *rw)
{
	int		result;

	if (rw->rw_magic != RW_MAGIC)
		return(EINVAL);

	if ( (result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
		return(result);

	//只要有读出者持有由调用者指定的读写锁的读出锁，或者有一个写入者持有该读写锁的唯一写入锁，调用线程就得阻塞
	while (rw->rw_refcount != 0) 
	{
		rw->rw_nwaitwriters++;
		result = pthread_cond_wait(&rw->rw_condwriters, &rw->rw_mutex);
		rw->rw_nwaitwriters--;
		if (result != 0)
			break;
	}
	if (result == 0)
		rw->rw_refcount = -1;

	pthread_mutex_unlock(&rw->rw_mutex);
	return(result);
}
/* end wrlock */

void Pthread_rwlock_wrlock(my_pthread_rwlock_t *rw)
{
	int		n;

	if ( (n = my_pthread_rwlock_wrlock(rw)) == 0)
		return;
	errno = n;
	err_sys("my_pthread_rwlock_wrlock error");
}
