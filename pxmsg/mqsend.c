#include	"unpipc.h"

int main(int argc, char **argv)
{
	mqd_t	mqd;
	void	*ptr;
	size_t	len;
	unsigned int prio;

	if (argc != 4)
		err_quit("usage: mqsend <name> <#bytes> <priority>");
	len = atoi(argv[2]);
	prio = atoi(argv[3]);

	mqd = Mq_open(argv[1], O_WRONLY);

	//num - 对象数目 size - 每个对象的大小 
	//为 len 个对象的数组分配内存，并初始化所有分配存储中的字节为零。
	//若分配成功，会返回指向分配内存块最低位（首位）字节的指针，它为任何类型适当地对齐。
	//若 sizeof(char) 为零，则行为是实现定义的（可返回空指针，或返回不可用于访问存储的非空指针）。 
	ptr = Calloc(len, sizeof(char));
	//从缓存ptr选取len长度发送到mqd描述符的文件
	//返回：若成功则为0，若出错则为-1
	Mq_send(mqd, ptr, len, prio);

	exit(0);
}
