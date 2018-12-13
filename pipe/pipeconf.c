#include	"unpipc.h"

int
main(int argc, char **argv)
{
	if (argc != 2)
		err_quit("usage: pipeconf <pathname>");
	//OPEN_MAX 一个进程在任意时刻打开的最大描述符数
	//PIPE_BUF 可原子地写往一个管道或FIFO的最大数据量
	printf("PIPE_BUF = %ld, OPEN_MAX = %ld\n", Pathconf(argv[1], _PC_PIPE_BUF), Sysconf(_SC_OPEN_MAX));
	exit(0);
}
