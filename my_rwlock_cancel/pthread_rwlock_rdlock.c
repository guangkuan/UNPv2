#include	"unpipc.h"
#include	"pthread_rwlock.h"

/* include rwlock_cancelrdwait */
static void rwlock_cancelrdwait(void *arg)
{
	my_pthread_rwlock_t	*rw;

	rw = arg;
	rw->rw_nwaitreaders--;
	pthread_mutex_unlock(&rw->rw_mutex);
}
/* end rwlock_cancelrdwait */

int my_pthread_rwlock_rdlock(my_pthread_rwlock_t *rw)
{
	int		result;

	if (rw->rw_magic != RW_MAGIC)
		return(EINVAL);

	if ( (result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
		return(result);

		/* 4give preference to waiting writers */
	while (rw->rw_refcount < 0 || rw->rw_nwaitwriters > 0) 
	{
		rw->rw_nwaitreaders++;
		//rw_lock_cancelrdwait是新建立的清理处理程序，它的单个参数将是读写锁指针rw
		pthread_cleanup_push(rwlock_cancelrdwait, (void *) rw);
		//若果pthread_rwlock_rdlock的调用线程在阻塞于该函数中的pthread_cond_wait调用期间被取消，它就不会从该函数返回，而是会调用清理处理程序
		result = pthread_cond_wait(&rw->rw_condreaders, &rw->rw_mutex);
		//删除清理处理程序
		//pthread_cleanup_pop的值为0的单个参数指示不再调用该处理程序，要是参数不为0，则再次调用这个处理程序再删除它
		pthread_cleanup_pop(0);
		rw->rw_nwaitreaders--;
		if (result != 0)
			break;
	}
	if (result == 0)
		rw->rw_refcount++;		/* another reader has a read lock */

	pthread_mutex_unlock(&rw->rw_mutex);
	return (0);
}

void my_Pthread_rwlock_rdlock(my_pthread_rwlock_t *rw)
{
	int		n;

	if ( (n = my_pthread_rwlock_rdlock(rw)) == 0)
		return;
	errno = n;
	err_sys("my_pthread_rwlock_rdlock error");
}
