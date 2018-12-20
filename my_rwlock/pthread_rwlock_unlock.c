/* include unlock */
#include	"unpipc.h"
#include	"pthread_rwlock.h"

int my_pthread_rwlock_unlock(my_pthread_rwlock_t *rw)
{
	int		result;

	if (rw->rw_magic != RW_MAGIC)
		return(EINVAL);

	if ( (result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
		return(result);
	
	if (rw->rw_refcount > 0) //有读出者
		rw->rw_refcount--;			/* releasing a reader */
	else if (rw->rw_refcount == -1)	//有写入者
		rw->rw_refcount = 0;		/* releasing a reader */
	else
		err_dump("rw_refcount = %d", rw->rw_refcount);

		/* 4give preference to waiting writers over waiting readers */
	//一旦有一个写入者在等待，我们就不给任何读出者授予读出锁，否则一个持续的读请求流可能永远阻塞某个等待着的写入者
	//因此判别写阻塞在前，读阻塞在后
	if (rw->rw_nwaitwriters > 0) 
	{
		if (rw->rw_refcount == 0) //只有一个写入者
			result = pthread_cond_signal(&rw->rw_condwriters);
	} 
	else if (rw->rw_nwaitreaders > 0)//读出者可能有多个
		result = pthread_cond_broadcast(&rw->rw_condreaders);

	pthread_mutex_unlock(&rw->rw_mutex);
	return(result);
}
/* end unlock */

void Pthread_rwlock_unlock(my_pthread_rwlock_t *rw)
{
	int		n;

	if ( (n = my_pthread_rwlock_unlock(rw)) == 0)
		return;
	errno = n;
	err_sys("my_pthread_rwlock_unlock error");
}
