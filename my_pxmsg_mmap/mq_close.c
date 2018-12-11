/* include mq_close */
#include	"unpipc.h"
#include	"mqueue.h"

int mymq_close(mymqd_t mqd)
{
	long	msgsize, filesize;
	struct mymq_hdr	*mqhdr;
	struct mymq_attr	*attr;
	struct mymq_info	*mqinfo;

	mqinfo = mqd;
	if (mqinfo->mqi_magic != MQI_MAGIC) 
	{
		errno = EBADF;
		return(-1);
	}
	mqhdr = mqinfo->mqi_hdr;
	attr = &mqhdr->mqh_attr;

	//调用mq_notify注销队列的调用进程
	if (mymq_notify(mqd, NULL) != 0)	/* unregister calling process */
		return(-1);

	msgsize = MSGSIZE(attr->mq_msgsize);
	filesize = sizeof(struct mymq_hdr) + (attr->mq_maxmsg * (sizeof(struct mymsg_hdr) + msgsize));
	if (munmap(mqinfo->mqi_hdr, filesize) == -1)
		return(-1);

	//为防止本函数的调用者在该内存区被malloc重用之前继续使用已关闭的消息队列描述符，把mqi_magic设置为0，这样我们的消息队列函数以后将检测到该错误
	mqinfo->mqi_magic = 0;		/* just in case */
	free(mqinfo);
	return(0);
}
/* end mq_close */

void Mymq_close(mymqd_t mqd)
{
	if (mymq_close(mqd) == -1)
		err_sys("mymq_close error");
}
