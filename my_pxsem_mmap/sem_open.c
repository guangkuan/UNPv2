/* include sem_open1 */
#include	"unpipc.h"
#include	"semaphore.h"

#include	<stdarg.h>		/* for variable arg lists */
#define		MAX_TRIES	10	/* for waiting for initialization */

mysem_t* mysem_open(const char *pathname, int oflag, ... )
{
	int		fd, i, created, save_errno;
	mode_t	mode;
	va_list	ap;
	mysem_t	*sem, seminit;
	struct stat	statbuff;
	unsigned int	value;
	pthread_mutexattr_t	mattr;
	pthread_condattr_t	cattr;

	created = 0;
	sem = MAP_FAILED;				/* [sic] */
again:
	if (oflag & O_CREAT) 
	{
		va_start(ap, oflag);		/* init ap to final named argument */
		mode = va_arg(ap, va_mode_t) & ~S_IXUSR;
		value = va_arg(ap, unsigned int);
		va_end(ap);

			/* 4open and specify O_EXCL and user-execute */
		fd = open(pathname, oflag | O_EXCL | O_RDWR, mode | S_IXUSR);
		if (fd < 0) 
		{
			if (errno == EEXIST && (oflag & O_EXCL) == 0)
				goto exists;		/* already exists, OK */
			else
				return(SEM_FAILED);
		}
		created = 1;
			/* 4first one to create the file initializes it */
			/* 4set the file size */
		//向文件写入sizeof(seminit)个0
		bzero(&seminit, sizeof(seminit));
		if (write(fd, &seminit, sizeof(seminit)) != sizeof(seminit))
			goto err;

			/* 4memory map the file */
		sem = mmap(NULL, sizeof(mysem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		if (sem == MAP_FAILED)
			goto err;

			/* 4initialize mutex, condition variable, and value */
		//初始化内存映射文件中的sem_t数据结构的三个成员：互斥锁、条件变量、信号量的值
		//因为Posix有名信号量可由知道其名字并由足够权限的任意进程共享，因此初始化时指定PTHREAD_PROCESS_SHARED属性
		if ( (i = pthread_mutexattr_init(&mattr)) != 0)
			goto pthreaderr;
		pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
		i = pthread_mutex_init(&sem->sem_mutex, &mattr);
		pthread_mutexattr_destroy(&mattr);	/* be sure to destroy */
		if (i != 0)
			goto pthreaderr;

		if ( (i = pthread_condattr_init(&cattr)) != 0)
			goto pthreaderr;
		pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
		i = pthread_cond_init(&sem->sem_cond, &cattr);
		pthread_condattr_destroy(&cattr);	/* be sure to destroy */
		if (i != 0)
			goto pthreaderr;

		//存入信号量初始值
		if ( (sem->sem_count = value) > sysconf(_SC_SEM_VALUE_MAX)) 
		{
			errno = EINVAL;
			goto err;
		}
			/* 4initialization complete, turn off user-execute bit */
		//完成信号量的初始化后，关掉用户执行位
		if (fchmod(fd, mode) == -1)
			goto err;
		close(fd);
		sem->sem_magic = SEM_MAGIC;
		return(sem);
	}
/* end sem_open1 */
/* include sem_open2 */
exists:
	if ( (fd = open(pathname, O_RDWR)) < 0) 
	{
		if (errno == ENOENT && (oflag & O_CREAT))
			goto again;
		goto err;
	}

	sem = mmap(NULL, sizeof(mysem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (sem == MAP_FAILED)
		goto err;

		/* 4make certain initialization is complete */
	for (i = 0; i < MAX_TRIES; i++) 
	{
		//调用stat查看内存映射文件的权限，如果它的用户执行位已关掉，那么该信号量已完成初始化
		if (stat(pathname, &statbuff) == -1) 
		{
			if (errno == ENOENT && (oflag & O_CREAT)) 
			{
				close(fd);
				goto again;
			}
			goto err;
		}
		if ((statbuff.st_mode & S_IXUSR) == 0) 
		{
			close(fd);
			sem->sem_magic = SEM_MAGIC;
			return(sem);
		}
		sleep(1);
	}
	errno = ETIMEDOUT;
	goto err;

pthreaderr:
	errno = i;
err:
		/* 4don't let munmap() or close() change errno */
	save_errno = errno;
	if (created)
		unlink(pathname);
	if (sem != MAP_FAILED)
		munmap(sem, sizeof(mysem_t));
	close(fd);
	errno = save_errno;
	return(SEM_FAILED);
}
/* end sem_open2 */

mysem_t* Mysem_open(const char *pathname, int oflag, ... )
{
	va_list	ap;
	mode_t	mode;
	mysem_t	*sem;
	unsigned int	value;

	if (oflag & O_CREAT) 
	{
		va_start(ap, oflag);		/* init ap to final named argument */
		mode = va_arg(ap, va_mode_t);
		value = va_arg(ap, unsigned int);
		va_end(ap);
		if ( (sem = mysem_open(pathname, oflag, mode, value)) == SEM_FAILED)
			err_sys("mysem_open error for %s", pathname);
	} 
	else 
	{
		if ( (sem = mysem_open(pathname, oflag)) == SEM_FAILED)
			err_sys("mysem_open error for %s", pathname);
	}

	return(sem);
}
