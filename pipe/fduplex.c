#include	"unpipc.h"

//这里全双工管道是由两个半双工管道构成的，写入fd[1]的数据只能从fd[0]读出，反之亦然
//父进程写入字符p，它由子进程读出，但此后父进程在试图从fd[1]read时中止，
//子进程则在试图往fd[0]write时中止
int main(int argc, char **argv)
{
	int		fd[2], n;
	char	c;
	pid_t	childpid;

	Pipe(fd);		/* assumes a full-duplex pipe (e.g., SVR4) */
	if ( (childpid = Fork()) == 0) 
	{		/* child */
		sleep(3);
		if ( (n = Read(fd[0], &c, 1)) != 1)
			err_quit("child: read returned %d", n);
		printf("child read %c\n", c);
		Write(fd[0], "c", 1);
		exit(0);
	}
		/* 4parent */
	//父进程往管道写入字符p
	Write(fd[1], "p", 1);
	//从管道读出一个字符
	if ( (n = Read(fd[1], &c, 1)) != 1)
		err_quit("parent: read returned %d", n);
	printf("parent read %c\n", c);
	exit(0);
}
