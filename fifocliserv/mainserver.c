#include	"fifo.h"

void	server(int, int);

int main(int argc, char **argv)
{
	int		readfifo, writefifo, dummyfd, fd;
	char	*ptr, buff[MAXLINE], fifoname[MAXLINE];
	pid_t	pid;
	ssize_t	n;

		/* 4create server's well-known FIFO; OK if already exists */
	if ((mkfifo(SERV_FIFO, FILE_MODE) < 0) && (errno != EEXIST))
		err_sys("can't create %s", SERV_FIFO);

		/* 4open server's well-known FIFO for reading and writing */
	readfifo = Open(SERV_FIFO, O_RDONLY, 0);
	//如果不以只写打开该FIFO，每当由一个客户终止时，该FIFO就变空，于是服务器的read返回0，表示是一个文件结束符。
	//然而如果我们总是有一个该FIFO的描述符打开用于写，那么当不在有客户存在时，服务器只是阻塞在read调用中，等待下一个客户请求。
	dummyfd = Open(SERV_FIFO, O_WRONLY, 0);		/* never used */

	//每个客户请求是由进程ID，一个空格再加路径名构成的
	while ( (n = Readline(readfifo, buff, MAXLINE)) > 0) 
	{
		if (buff[n-1] == '\n')
			n--;			/* delete newline from readline() */
		buff[n] = '\0';		/* null terminate pathname */

		//由strchr函数返回的赋给ptr的指针指向客户请求行中的空格
		if ( (ptr = strchr(buff, ' ')) == NULL) 
		{
			err_msg("bogus request: %s", buff);
			continue;
		}

		//ptr增1后即指向后面跟随的路径名的首字母
		*ptr++ = 0;			/* null terminate PID, ptr = pathname */
		pid = atol(buff);
		snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%ld", (long) pid);
		if ( (writefifo = open(fifoname, O_WRONLY, 0)) < 0) 
		{
			err_msg("cannot open: %s", fifoname);
			continue;
		}

		if ( (fd = open(ptr, O_RDONLY)) < 0) 
		{
				/* 4error: must tell client */
			snprintf(buff + n, sizeof(buff) - n, ": can't open, %s\n",
					 strerror(errno));
			n = strlen(ptr);
			Write(writefifo, ptr, n);
			Close(writefifo);
	
		} 
		else 
		{
				/* 4open succeeded: copy file to FIFO */
			while ( (n = Read(fd, buff, MAXLINE)) > 0)
				Write(writefifo, buff, n);
			Close(fd);
			//完成后close客户的FIFO的服务器端，以使得客户的read返回0（文件结束符）
			Close(writefifo);
		}
	}
}
