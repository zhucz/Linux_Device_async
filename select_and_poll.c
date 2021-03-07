 int select(int numfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
			struct timeval *timeout);
			
			
//下列操作用来设置、清除、判断文件描述符集合：
FD_ZERO(fd_set *set)

FD_SET(int fd, fd_set *set)

FD_ISSET(int fd, fd_set *set)

// pool()的功能和实现原理与select()相似
int poll(struct pollfd *fds, nfds_t nfds, int timeout);


//多路复用的文件数量庞大、I/O流量频繁的时候，一般不太适合使用
//select()和pol(),此种情况下，select()和poll()的性能表现较差，我们
//宜使用epoll。epoll的最大好处是不会随着fd的数目增长而降低效率，
//select()则会随着fd的数量增大性能下降明显。

//与epoll相关的用户空间编程接口包括
int epoll_create(int size);

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

epfd: epoll的返回值
op:EPOLL_CTL_ADD:注册新的fd到epfd中
   EPOLL_CTL_MOD
   EPOL_CTL_DEL
fd:是需要监听的fd
struct epoll_event 告诉内核需要监听的事件类型

struct epoll_event {
	__uint32_t events;
	epoll_data_t data;
}

EPOLLIN:
EPOLLOUT
EPOLLPRI
EPOLLERR
EPOLHUP
EPOLLET


int epoll_wait(int epfd, struct epoll_event *events,int maxevents, int timeout);


//设备驱动中的poll函数
unsigned int (*poll)(struct file *filp, struct poll_table *wait);


void poll_wait(struct file *filp, wait_queue_heat_t *queue, poll_table *wait);
//poll_wait不会引起阻塞

//设备驱动中poll()函数的典型模板

static unsigned int xxx_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask = 0;
	struct xxx_dev *dev = filp->private_data;//获取设备结构体指针
	
	....
	poll_wait(filp, &dev->r_wait, wait);//加入读等待队列
	poll_wait(filp, &dev->w_wait, wait);//加入写等待队列
	
	if(....)							//可读
		mask |= POLLIN | POLLRDNORM;	//表示数据可获得（对用户可读——

	if(....)							//可写
		mask |= POLLOUT | POLLWRNORM;	//表示数据可写
	
	.....
	
	return mask;
		
}


static unsigned int globalfifo_poll(struct file *filp, poll_table *wait)
{
	unsigned int mask = 0;
	struct globalfifo_dev *dev = filp->private_data;
	
	mutex_lock(&dev->mutex);
	
	poll_wait(filp, &dev->r_wait, wait);
	poll_wait(filp, &dev->w_wait, wait);
	
	if(dev->current_len != 0){
		mask |= POLLIN | POLLRDNORM;
	}
	
	if(dev->current_len != GLOBALFIFO_SIZE){
		mask |= POLLOUT |POLLWRNORM;
	}
	
	mutex_unlock(&dev->mutex);
	return mask;
}

static const struct file_operations globalfifo_fops = {
	....
	.poll = globalfifo_poll,
	....
};

//example  select()
#incldue <stdio.h>
#define FIFO_CLEAR	0x1
#define BUFFER_LEN	20


void main(void)
{
	int fd, num;
	char rd_ch[BUFFER_LEN];
	
	fd_set rfds, wfds;  /*读/写文件描述符集*/
	//以非组赛方式打开设备
	fd = open("/dev/globalfifo",O_RDONLY | O_NONBLOCK);
	if(fd != -1){
		//FIFO 清 0
		if(ioctl(fd, FIFO_CLEAR, 0) < 0)
			printf("ioctl command failed \n");
		
		while(1){
			FD_ZERO(&rfds);
			FD_ZERO(&wfds);
			FD_SET(fd, &rfds);
			FD_SET(fd, &wfds);
			
			select(fd +1, &rfds, &wfds, NULL, NULL);
			//数据可获得
			if(FD_ISSET(fd, &rfds)
				printf("poll monitor:can be read\");
			//数据可写入
			if(FD_ISSET(fd, &wfds);
				printf("poll monitor:can be written \n");
		}
	}else{
		printf("Device open failure \n");
	}
}



//example epoll_create





//在设备驱动中阻塞I/O一般基于等待队列或者基于等待队列的其他Linux内核API来实现














