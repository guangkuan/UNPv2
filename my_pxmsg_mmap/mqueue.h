/* include mqueueh */
//消息队列描述符只是一个指向某个mq_info结构的指针
//消息队列描述符不必像文件描述符那样是一个小整数，唯一的Posix要求是这种数据类型不能是一个数组类型
typedef struct mymq_info	*mymqd_t;		/* opaque datatype */

//创建Posix消息队列时传入的第四个参数。
struct mymq_attr {
  long	mq_flags;		/* message queue flag: O_NONBLOCK */
  long	mq_maxmsg;		/* max number of messages allowed on queue */
  long	mq_msgsize;		/* max size of a message (in bytes) */
  long	mq_curmsgs;		/* number of messages currently on queue */
};

	/* 4one mymq_hdr{} per queue, at beginning of mapped file */
//该结构出现在映射文件的开头
struct mymq_hdr {
  struct mymq_attr	mqh_attr;	/* the queue's attributes */
  long				mqh_head;	/* index of first message 含有当前在队列中的所有消息的链表*/
  long				mqh_free;	/* index of first free message 含有队列中的所有空闲消息的链表*/
  long				mqh_nwait;	/* #threads blocked in mq_receive() */
  pid_t				mqh_pid;	/* nonzero PID if mqh_event set */
  struct sigevent	mqh_event;	/* for mq_notify() */
  pthread_mutex_t	mqh_lock;	/* mutex lock */
  pthread_cond_t	mqh_wait;	/* and condition variable */
};

	/* 4one mymsg_hdr{} at the front of each message in the mapped file */
//该结构出现在映射文件中每个消息的开头
struct mymsg_hdr {
  long	msg_next;				/* index of next on linked list */
								/* 4msg_next must be first member in struct */
  ssize_t	msg_len;			/* actual length */
  unsigned int	msg_prio;		/* priority */
};

	/* 4one mymq_info{} malloc'ed per process per mq_open() */
//对应消息队列描述符类型
struct mymq_info {
  struct mymq_hdr	*mqi_hdr;	/* start of mmap'ed region */
  long	mqi_magic;				/* magic number if open */
  int	mqi_flags;				/* flags for this process */
};
#define	MQI_MAGIC	0x98765432

	/* 4size of message in file is rounded up for alignment */
#define	MSGSIZE(i)	((((i) + sizeof(long)-1) / sizeof(long)) * sizeof(long))
/* end mqueueh */

	/* 4our functions */
int		 mymq_close(mymqd_t);
int		 mymq_getattr(mymqd_t, struct mymq_attr *);
int		 mymq_notify(mymqd_t, const struct sigevent *);
mymqd_t	 mymq_open(const char *, int, ...);
ssize_t	 mymq_receive(mymqd_t, char *, size_t, unsigned int *);
int		 mymq_send(mymqd_t, const char *, size_t, unsigned int);
int		 mymq_setattr(mymqd_t, const struct mymq_attr *, struct mymq_attr *);
int		 mymq_unlink(const char *name);

	/* 4and the corresponding wrapper functions */
void	 Mymq_close(mymqd_t);
void	 Mymq_getattr(mymqd_t, struct mymq_attr *);
void	 Mymq_notify(mymqd_t, const struct sigevent *);
mymqd_t	 Mymq_open(const char *, int, ...);
ssize_t	 Mymq_receive(mymqd_t, char *, size_t, unsigned int *);
void	 Mymq_send(mymqd_t, const char *, size_t, unsigned int);
void	 Mymq_setattr(mymqd_t, const struct mymq_attr *, struct mymq_attr *);
void	 Mymq_unlink(const char *name);
