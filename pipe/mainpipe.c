#include	"unpipc.h"

void	client(int, int), server(int, int);

//客户作为父进程运行，服务器则作为子进程运行。
//第一个管道用于从客户向服务器发送路径名，第二个管道用于从服务器向客户发送该文件的内容
int main(int argc, char **argv)
{
	int		pipe1[2], pipe2[2];
	pid_t	childpid;

	Pipe(pipe1);	/* create two pipes */
	Pipe(pipe2);

	if ( (childpid = Fork()) == 0) 
	{		/* child */
		Close(pipe1[1]);
		Close(pipe2[0]);

		//读1，写2
		server(pipe1[0], pipe2[1]);
		//子进程往管道写入最终数据后调用exit首先终止，随后编程一个僵尸进程
		exit(0);
	}
		/* 4parent */
	Close(pipe1[0]);
	Close(pipe2[1]);

	//读2，写1
	client(pipe2[0], pipe1[1]);

	//负进程调用waitpid取得已终止子进程的终止状态
	Waitpid(childpid, NULL, 0);		/* wait for child to terminate */
	exit(0);
}
