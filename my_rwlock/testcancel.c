#include	"unpipc.h"
#include	"pthread_rwlock.h"

my_pthread_rwlock_t	rwlock = PTHREAD_RWLOCK_INITIALIZER;

void	 *thread1(void *), *thread2(void *);
pthread_t	tid1, tid2;

int main(int argc, char **argv)
{
	void	*status;
	Pthread_rwlock_init(&rwlock, NULL);

	Set_concurrency(2);
	Pthread_create(&tid1, NULL, thread1, NULL);
	sleep(1);		/* let thread1() get the lock */
	Pthread_create(&tid2, NULL, thread2, NULL);

	Pthread_join(tid2, &status);
	if (status != PTHREAD_CANCELED)
		printf("thread2 status = %p\n", status);
	Pthread_join(tid1, &status);
	if (status != NULL)
		printf("thread1 status = %p\n", status);

	printf("rw_refcount = %d, rw_nwaitreaders = %d, rw_nwaitwriters = %d\n",
		   rwlock.rw_refcount, rwlock.rw_nwaitreaders,
		   rwlock.rw_nwaitwriters);
	Pthread_rwlock_destroy(&rwlock);
		/* 4returns EBUSY error if cancelled thread does not cleanup */

	exit(0);
}

void* thread1(void *arg)
{
	Pthread_rwlock_rdlock(&rwlock);
	printf("thread1() got a read lock\n");
	//有一个读出锁活跃期间，是无法提供写入锁的
	sleep(3);		/* let thread2 block in my_pthread_rwlock_wrlock() */
	//调用pthread_cancel取消另一个线程
	//当阻塞在某个条件变量等待中的一个线程被取消时，要再次取得与该条件变量关联的互斥锁，然后调用第一个线程取消清理处理程序
	//我们尚未安装任何线程取消清理处理程序，但是所关联的互斥锁仍然在该线程被取消前再次取得
	//因此，当第二个线程被取消时，它持有包含在读写锁中的互斥锁，而rw_nwaitwriters的值已被加1
	pthread_cancel(tid2);
	sleep(3);
	//释放所有的读出锁
	//第一个线程调用pthread_rwlock_unlock，但它永远阻塞在其中的pthread_mutex_lock调用中，因为它想要持有的互斥锁仍然由已被取消的线程锁着
	Pthread_rwlock_unlock(&rwlock);
	return(NULL);
}

void* thread2(void *arg)
{
	printf("thread2() trying to obtain a write lock\n");
	Pthread_rwlock_wrlock(&rwlock);
	printf("thread2() got a write lock\n");
	sleep(1);
	Pthread_rwlock_unlock(&rwlock);
	return(NULL);
}
