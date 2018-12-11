#include	"unpipc.h"

#define	MESGSIZE	256		/* max #bytes per message, incl. null at end */
#define	NMESG		 16		/* max #messages */

struct shmstruct {		/* struct stored in shared memory */
  sem_t	mutex;			/* three Posix memory-based semaphores */
  sem_t	nempty;
  sem_t	nstored;
  int	nput;			/* index into msgoff[] for next put */
  long	noverflow;		/* #overflows by senders */
  sem_t	noverflowmutex;	/* mutex for noverflow counter */
  //共享内存区对象和映射到映射它的各个进程的不同物理地址。也就是说，对于同一个共享内存区对象，调用mmp的每个进程所得到的mmqp返回值可能不同
  long	msgoff[NMESG];	/* offset in shared memory of each message */
  char	msgdata[NMESG * MESGSIZE];	/* the actual messages */
};
