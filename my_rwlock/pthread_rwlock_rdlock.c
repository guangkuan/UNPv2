/* include rdlock */
#include	"unpipc.h"
#include	"pthread_rwlock.h"

int my_pthread_rwlock_rdlock(my_pthread_rwlock_t *rw)
{
	int		result;

	if (rw->rw_magic != RW_MAGIC)
		return(EINVAL);

	if ( (result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
		return(result);

		/* 4give preference to waiting writers */
	//如果rw_refcount小于0（意味着当前有一个写入者持有由调用者指定的读写锁（写入锁））
	//如果rw_nwaitwaiters大于0（意味着有线程正在等着获取该读写锁的一个写入锁（阻塞了一个写入，该例子优先写入））因此我们无法获取该读写锁的一个读出锁
	while (rw->rw_refcount < 0 || rw->rw_nwaitwriters > 0) 
	{
		rw->rw_nwaitreaders++;
		//阻塞当前的读操作
		result = pthread_cond_wait(&rw->rw_condreaders, &rw->rw_mutex);
		rw->rw_nwaitreaders--;
		if (result != 0)
			break;
	}
	if (result == 0)
		rw->rw_refcount++;		/* another reader has a read lock */

	pthread_mutex_unlock(&rw->rw_mutex);
	return (result);
}
/* end rdlock */

void Pthread_rwlock_rdlock(my_pthread_rwlock_t *rw)
{
	int		n;

	if ( (n = my_pthread_rwlock_rdlock(rw)) == 0)
		return;
	errno = n;
	err_sys("my_pthread_rwlock_rdlock error");
}
