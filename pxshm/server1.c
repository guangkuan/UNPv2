#include	"unpipc.h"

struct shmstruct {	/* struct stored in shared memory */
  int	count;
};
sem_t	*mutex;		/* pointer to named semaphore */

int
main(int argc, char **argv)
{
	int		fd;
	struct shmstruct	*ptr;

	if (argc != 3)
		err_quit("usage: server1 <shmname> <semname>");
	//调用shm_unlink以提防所需共享内存区对象已经存在的情况
	shm_unlink(Px_ipc_name(argv[1]));		/* OK if this fails */
		/* 4create shm, set its size, map it, close descriptor */
	fd = Shm_open(Px_ipc_name(argv[1]), O_RDWR | O_CREAT | O_EXCL, FILE_MODE);
	Ftruncate(fd, sizeof(struct shmstruct));
	ptr = Mmap(NULL, sizeof(struct shmstruct), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	Close(fd);

	////调用sem_unlink以提防所需信号量已经存在的情况
	sem_unlink(Px_ipc_name(argv[2]));		/* OK if this fails */
	//将给存放在所创建的共享内存区对象中的计数器加1的任何进程都会把该信号量用作一个互斥锁
	mutex = Sem_open(Px_ipc_name(argv[2]), O_CREAT | O_EXCL, FILE_MODE, 1);
	Sem_close(mutex);

	exit(0);
}
