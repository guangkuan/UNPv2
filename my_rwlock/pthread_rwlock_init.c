/* include init */
#include	"unpipc.h"
#include	"pthread_rwlock.h"
#include <stddef.h>
#include <asm-generic/errno-base.h>
#include <errno.h>

//读写锁规则
//只要没有线程持有某个给定的读写锁用于写，那么任意数目的线程可以持有该读写锁用于读
//仅当没有线程持有某个给定的读写锁用于读或用于写时，才能分配该读写锁用于写
int my_pthread_rwlock_init(my_pthread_rwlock_t *rw, my_pthread_rwlockattr_t *attr)
{
	int		result;

	if (attr != NULL)
		return(EINVAL);		/* not supported */

	if ( (result = pthread_mutex_init(&rw->rw_mutex, NULL)) != 0)
		goto err1;
	if ( (result = pthread_cond_init(&rw->rw_condreaders, NULL)) != 0)
		goto err2;
	if ( (result = pthread_cond_init(&rw->rw_condwriters, NULL)) != 0)
		goto err3;
	rw->rw_nwaitreaders = 0;
	rw->rw_nwaitwriters = 0;
	rw->rw_refcount = 0;
	//rw_magic成员设置为表示该结构已初始化完毕的值
	rw->rw_magic = RW_MAGIC;

	return(0);

err3:
	pthread_cond_destroy(&rw->rw_condreaders);
err2:
	pthread_mutex_destroy(&rw->rw_mutex);
err1:
	return(result);			/* an errno value */
}
/* end init */

void Pthread_rwlock_init(my_pthread_rwlock_t *rw, my_pthread_rwlockattr_t *attr)
{
	int		n;

	if ( (n = my_pthread_rwlock_init(rw, attr)) == 0)
		return;
	errno = n;
	err_sys("my_pthread_rwlock_init error");
}
