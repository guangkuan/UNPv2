#include	"unpipc.h"

int main(int argc, char **argv)
{
	size_t	n;
	char	buff[MAXLINE], command[MAXLINE];
	FILE	*fp;

		/* 4read pathname */
	Fgets(buff, MAXLINE, stdin);
	n = strlen(buff);		/* fgets() guarantees null byte at end */
	if (buff[n-1] == '\n')
		n--;				/* delete newline from fgets() */

	snprintf(command, sizeof(command), "cat %s", buff);
	//popen在调用进程和所指定的命令之间创建一个管道
	//由popen返回的值是一个标准I/O FILE指针，该指针用于输入还是输出取决于字符串type("r")
	fp = Popen(command, "r");

		/* 4copy from pipe to standard output */
	while (Fgets(buff, MAXLINE, fp) != NULL)
		Fputs(buff, stdout);

	Pclose(fp);
	exit(0);
}
