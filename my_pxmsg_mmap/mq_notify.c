/* include mq_notify */
#include	"unpipc.h"
#include	"mqueue.h"

int mymq_notify(mymqd_t mqd, const struct sigevent* notification)
{
	int		n;
	pid_t	pid;
	struct mymq_hdr	*mqhdr;
	struct mymq_info	*mqinfo;

	mqinfo = mqd;
	if (mqinfo->mqi_magic != MQI_MAGIC) 
	{
		errno = EBADF;
		return(-1);
	}
	mqhdr = mqinfo->mqi_hdr;
	if ( (n = pthread_mutex_lock(&mqhdr->mqh_lock)) != 0) 
	{
		errno = n;
		return(-1);
	}

	pid = getpid();
	//结合mq_send看
	//如果第二个参数是个空指针，那么注销所指定队列的调用进程
	if (notification == NULL) 
	{
		if (mqhdr->mqh_pid == pid) 
		{
			mqhdr->mqh_pid = 0;	/* unregister calling process */
		} 						/* no error if caller not registered */
	} 
	else 
	{
		//如果某个进程已被注册，我们就通过向它发送信号0（成为空信号（null signal））以检查它是否仍然存在
		if (mqhdr->mqh_pid != 0) 
		{
			//该进程不存在时会返回一个ESRCH错误，仍存在就返回一个EBUSY错误
			if (kill(mqhdr->mqh_pid, 0) != -1 || errno != ESRCH) 
			{
				errno = EBUSY;
				goto err;
			}
		}
		mqhdr->mqh_pid = pid;
		mqhdr->mqh_event = *notification;
	}
	pthread_mutex_unlock(&mqhdr->mqh_lock);
	return(0);

err:
	pthread_mutex_unlock(&mqhdr->mqh_lock);
	return(-1);
}
/* end mq_notify */

void Mymq_notify(mymqd_t mqd, const struct sigevent *notification)
{
	if (mymq_notify(mqd, notification) == -1)
		err_sys("mymq_notify error");
}
