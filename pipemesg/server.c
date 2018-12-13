#include	"mesg.h"

void server(int readfd, int writefd)
{
	FILE	*fp;
	ssize_t	n;
	struct mymesg	mesg;

		/* 4read pathname from IPC channel */
	mesg.mesg_type = 1;
	if ( (n = Mesg_recv(readfd, &mesg)) == 0)
		err_quit("pathname missing");
	mesg.mesg_data[n] = '\0';	/* null terminate pathname */

	if ( (fp = fopen(mesg.mesg_data, "r")) == NULL) 
	{
			/* 4error: must tell client */
		snprintf(mesg.mesg_data + n, sizeof(mesg.mesg_data) - n, ": can't open, %s\n", strerror(errno));
		mesg.mesg_len = strlen(mesg.mesg_data);
		Mesg_send(writefd, &mesg);

	} 
	else 
	{
			/* 4fopen succeeded: copy file to IPC channel */
		//发送消息中包含换行符
		//如果fopen调用成功，就使用fgets读出该文件并发送给客户，每个消息一行。
		while (Fgets(mesg.mesg_data, MAXMESGDATA, fp) != NULL) 
		{
			mesg.mesg_len = strlen(mesg.mesg_data);
			Mesg_send(writefd, &mesg);
		}
		Fclose(fp);
	}

		/* 4send a 0-length message to signify the end */
	//可通过发送回一个长度位0的消息来通知对端已到达输入文件的结尾
	mesg.mesg_len = 0;
	Mesg_send(writefd, &mesg);
}
