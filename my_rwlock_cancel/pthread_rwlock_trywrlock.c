/* include trywrlock */
#include	"unpipc.h"
#include	"pthread_rwlock.h"

int
my_pthread_rwlock_trywrlock(my_pthread_rwlock_t *rw)
{
	int		result;

	if (rw->rw_magic != RW_MAGIC)
		return(EINVAL);

	if ( (result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
		return(result);

	if (rw->rw_refcount != 0)
		result = EBUSY;			/* held by either writer or reader(s) */
	else
		rw->rw_refcount = -1;	/* available, indicate a writer has it */

	pthread_mutex_unlock(&rw->rw_mutex);
	return(result);
}
/* end trywrlock */

int
my_Pthread_rwlock_trywrlock(my_pthread_rwlock_t *rw)
{
	int		n;

	if ( (n = my_pthread_rwlock_trywrlock(rw)) != 0) {
		errno = n;
		err_sys("my_pthread_rwlock_trywrlock error");
	}
	return(n);
}
