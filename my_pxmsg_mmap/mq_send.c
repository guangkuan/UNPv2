/* include mq_send1 */
#include	"unpipc.h"
#include	"mqueue.h"

int mymq_send(mymqd_t mqd, const char *ptr, size_t len, unsigned int prio)
{
	int		n;
	long	index, freeindex;
	int8_t	*mptr;
	struct sigevent	*sigev;
	struct mymq_hdr	*mqhdr;
	struct mymq_attr	*attr;
	struct mymsg_hdr	*msghdr, *nmsghdr, *pmsghdr;
	struct mymq_info	*mqinfo;

	mqinfo = mqd;
	if (mqinfo->mqi_magic != MQI_MAGIC) 
	{
		errno = EBADF;
		return(-1);
	}
	mqhdr = mqinfo->mqi_hdr;	/* struct pointer */
	mptr = (int8_t *) mqhdr;	/* byte pointer */
	attr = &mqhdr->mqh_attr;
	if ( (n = pthread_mutex_lock(&mqhdr->mqh_lock)) != 0) 
	{
		errno = n;
		return(-1);
	}

	//检查待发送消息，确定其大小没有超过该队列的最大消息大小
	if (len > attr->mq_msgsize) 
	{
		errno = EMSGSIZE;
		goto err;
	}
	//检查是空队列中放置信息，结合mq_notify看
	if (attr->mq_curmsgs == 0) 
	{
		//检查是否有某个进程被注册为接收出自该队列的通知，并检查是否有某个线程阻塞在mq_receive调用中
		if (mqhdr->mqh_pid != 0 && mqhdr->mqh_nwait == 0) 
		{
			sigev = &mqhdr->mqh_event;
			if (sigev->sigev_notify == SIGEV_SIGNAL) 
			{
				sigqueue(mqhdr->mqh_pid, sigev->sigev_signo, sigev->sigev_value);
			}
			//已注册了的进程随后被注销
			mqhdr->mqh_pid = 0;		/* unregister */
		}
	} 
	//如果调用者指定的队列已填满
	else if (attr->mq_curmsgs >= attr->mq_maxmsg) 
	{
			/* 4queue is full */
		if (mqinfo->mqi_flags & O_NONBLOCK) 
		{
			errno = EAGAIN;
			goto err;
		}
			/* 4wait for room for one message on the queue */
		//如果阻塞，等待条件变量mqh_wait
		while (attr->mq_curmsgs >= attr->mq_maxmsg)
			pthread_cond_wait(&mqhdr->mqh_wait, &mqhdr->mqh_lock);
	}
/* end mq_send1 */
/* include mq_send2 */
		/* 4nmsghdr will point to new message */
	if ( (freeindex = mqhdr->mqh_free) == 0)
		err_dump("mymq_send: curmsgs = %ld; free = 0", attr->mq_curmsgs);
	
	//初始时，消息队列中所有的消息都是空闲状态，也就是说这个消息中不包含有效数据。当放置一个消息到消息队列中时，我们挑选一个空闲状态的消息，把数据放入，这条消息的状态就变成非空闲的了。
	//nmsghdr含有所有映射内存区中用于存放待写入消息的位置的地址
	nmsghdr = (struct mymsg_hdr *) &mptr[freeindex];
	//优先级
	nmsghdr->msg_prio = prio;
	nmsghdr->msg_len = len;
	//这里把消息内容存入
	memcpy(nmsghdr + 1, ptr, len);		/* copy message from caller */
	mqhdr->mqh_free = nmsghdr->msg_next;	/* new freelist head */

		/* 4find right place for message in linked list */
	index = mqhdr->mqh_head;
	//pmsghdr含有链表中上一个消息的地址
	//这里相当于更新后面的链表
	pmsghdr = (struct mymsg_hdr *) &(mqhdr->mqh_head);
	while (index != 0) 
	{
		msghdr = (struct mymsg_hdr *) &mptr[index];
		if (prio > msghdr->msg_prio) 
		{
			nmsghdr->msg_next = index;
			pmsghdr->msg_next = freeindex;
			break;
		}
		index = msghdr->msg_next;
		pmsghdr = msghdr;
	}
	if (index == 0) 
	{
			/* 4queue was empty or new goes at end of list */
		pmsghdr->msg_next = freeindex;
		nmsghdr->msg_next = 0;
	}
		/* 4wake up anyone blocked in mq_receive waiting for a message */
	if (attr->mq_curmsgs == 0)
		pthread_cond_signal(&mqhdr->mqh_wait);
	attr->mq_curmsgs++;

	pthread_mutex_unlock(&mqhdr->mqh_lock);
	return(0);

err:
	pthread_mutex_unlock(&mqhdr->mqh_lock);
	return(-1);
}
/* end mq_send2 */

void Mymq_send(mymqd_t mqd, const char *ptr, size_t len, unsigned int prio)
{
	if (mymq_send(mqd, ptr, len, prio) == -1)
		err_sys("mymq_send error");
}
