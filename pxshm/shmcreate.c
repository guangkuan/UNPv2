#include	"unpipc.h"

int main(int argc, char **argv)
{
	int		c, fd, flags;
	char	*ptr;
	off_t	length;

	flags = O_RDWR | O_CREAT;
	while ( (c = Getopt(argc, argv, "e")) != -1) 
	{
		switch (c) 
		{
		case 'e':
			flags |= O_EXCL;
			break;
		}
	}
	if (optind != argc - 2)
		err_quit("usage: shmcreate [ -e ] <name> <length>");
	length = atoi(argv[optind + 1]);

	//文件存在/dev/shm
	//共享内存区对象(shared memory object)与12章内存映射文件(memory mapped file)不同，用shm_open获取描述符
	//shm_open的返回值是一个整数描述符
	fd = Shm_open(argv[optind], flags, FILE_MODE);
	Ftruncate(fd, length);

	ptr = Mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	exit(0);
}
