#include	"unpipc.h"
/* include rwlockh */
#ifndef	__pthread_rwlock_h
#define	__pthread_rwlock_h

//读写锁的原型通常在<pthread.h>头文件中
typedef struct {
  pthread_mutex_t	rw_mutex;		/* basic lock on this struct 互斥锁*/
  pthread_cond_t	rw_condreaders;	/* for reader threads waiting 条件变量*/
  pthread_cond_t	rw_condwriters;	/* for writer threads waiting 条件变量*/
  //该结构初始化成功后，标志成员rw_magic就被设置成RW_MAGIC。
  //所有成员都测试该成员，以检查调用者是否向某个已初始化的读写锁传递了指针
  //读写锁被摧毁时，这个成员就被置为0
  int				rw_magic;		/* for error checking 标志*/
  int				rw_nwaitreaders;/* the number waiting 计数器*/
  int				rw_nwaitwriters;/* the number waiting 计数器*/
  //rw_refcount指示着本读写锁当前的状态
  //-1表示它是一个写入锁
  //0表示它是可用的
  //大于0表示它当前容纳那么多的读出锁
  int				rw_refcount;/*计数器*/
	/* 4-1 if writer has the lock, else # readers holding the lock */
} my_pthread_rwlock_t;

#define	RW_MAGIC	0x19283746

	/* 4following must have same order as elements in struct above */
//读写锁数据类型为pthread_rwlock_t，如果这个类型的某个变量是静态分配的，那么可通过给它赋常值PTHREAD_RWLOCK_INITIALIZER来初始化
#define	PTHREAD_RWLOCK_INITIALIZER	{ PTHREAD_MUTEX_INITIALIZER, \
			PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, \
			RW_MAGIC, 0, 0, 0 }

typedef	int		my_pthread_rwlockattr_t;	/* dummy; not supported */

	/* 4function prototypes */
int		my_pthread_rwlock_destroy(my_pthread_rwlock_t *);
int		my_pthread_rwlock_init(my_pthread_rwlock_t *, my_pthread_rwlockattr_t *);
int		my_pthread_rwlock_rdlock(my_pthread_rwlock_t *);
int		my_pthread_rwlock_tryrdlock(my_pthread_rwlock_t *);
int		my_pthread_rwlock_trywrlock(my_pthread_rwlock_t *);
int		my_pthread_rwlock_unlock(my_pthread_rwlock_t *);
int		my_pthread_rwlock_wrlock(my_pthread_rwlock_t *);
/* $$.bp$$ */
	/* 4and our wrapper functions */
void	Pthread_rwlock_destroy(my_pthread_rwlock_t *);
void	Pthread_rwlock_init(my_pthread_rwlock_t *, my_pthread_rwlockattr_t *);
void	Pthread_rwlock_rdlock(my_pthread_rwlock_t *);
int		Pthread_rwlock_tryrdlock(my_pthread_rwlock_t *);
int		Pthread_rwlock_trywrlock(my_pthread_rwlock_t *);
void	Pthread_rwlock_unlock(my_pthread_rwlock_t *);
void	Pthread_rwlock_wrlock(my_pthread_rwlock_t *);

#endif	__pthread_rwlock_h
/* end rwlockh */
