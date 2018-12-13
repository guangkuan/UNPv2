#include	"mesg.h"

void client(int readfd, int writefd)
{
	size_t	len;
	ssize_t	n;
	struct mymesg	mesg;

		/* 4read pathname */
	Fgets(mesg.mesg_data, MAXMESGDATA, stdin);
	len = strlen(mesg.mesg_data);
	if (mesg.mesg_data[len-1] == '\n')
		len--;				/* delete newline from fgets() */
	mesg.mesg_len = len;
	mesg.mesg_type = 1;

		/* 4write pathname to IPC channel */
	Mesg_send(writefd, &mesg);

		/* 4read from IPC, write to standard output */
	//mesg_recv返回一个值为0的长度表示已到达来自服务器的数据的结尾
	//服务器将在发送给客户的每个消息中都包含换行符，因此空行也会有一个值为1的消息长度
	while ( (n = Mesg_recv(readfd, &mesg)) > 0)
		Write(STDOUT_FILENO, mesg.mesg_data, n);
}
