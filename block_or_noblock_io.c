//等待队列:实现阻塞进程的唤醒，用于同步对系统资源的访问
//信号量依赖于等待队列实现
//1.定义等待队列
wait_queue_head_t  my_queue;

wait_queue_head_t 是__wati_queue_head 结构体的一个typedef

//2.初始化 ”等待队列头部”
init_waitqueue_head(&my_queue);

//定义并初始化等待队列头部“快捷方式”
DECLARE_WAIT_QUEUE_HEAD(name);

//3.定义等待队列元素
DECLARE_WAITQUEUE(name, tsk);//定义并初始化一个名为name的等待队列元素。


//4.添加/移除等待队列
//用于将等待队列元素wait添加到等待队列头部 q 指向的双向链表中
void add_wait_queue(wait_queue_head_t *q, wait_queue_t *wait);

//用于将等待队列元素wait从由 q 头部指向的链表中删除
void remove_wait_queue(wait_queue_head_t *q, wait_queue_t *wait);


//5.等待事件
wait_event(queue, condition);						//不可以信号打断
wait_event_interruptible(queue, condition);//可以被信号打断

wait_event_timeout(queue, condition, timeout);//阻塞但是有超时时间以jiffy为单位
wait_event_interruptible_timeout(queue, condition, timeout);


//6.唤醒队列 唤醒以queue作为等待队列头部的队列中所有的进程
void wake_up(wait_queue_head_t *queue);
void wake_up_interruptible(wait_queue_head_t *queue);


wake_up()应该与wait_event()或者wait_event_timeout()成对使用
wake_up_interruptible()应该与wait_event_interruptible()或者wait_event_interruptible_timeout()成对使用

//7.在等待队列上睡眠
sleep_on()函数作用就是将目前进程的状态设置成     		TASK_UNINTERRUPTIBLE

interruptible_sleep_on()函数将进程状态设置成 TASK_INTERRUPTIBLE


//example 在设备驱动中使用等待队列
static ssize_t xxx_write(struct file *filp, const char *buffer, size_t count, loff_t *ppos)
{
	.....
	DECLARE_WAITQUEUE(wait, current);//定义等待队列元素
	add_wait_queue(&xxx_wait, &wait);//添加元素到等待队列

	/*等待设备缓冲区可写*/

	do{
		avail = device_writable{....};
		if(avail < 0){
			if(file->f_flags & O_NONBLOCK){			//非阻塞
				ret = -EAGAIN;
				goto out;
			}
			__set_current_state(TASK_INTERRUPIBLE);	//改变进程状态
			schedule();								//调度其他进程执行
			if(signal_pending(current)){			//如果是因为信号唤醒
				ret = -ERESTARTSYS;
				goto out;
			}
		}
	
	}while(avail < 0);

	/*写设备缓冲区*/
	device_wirte{......}

	out:
		remove_wait_queue(&xxx_wait, &wait);//将元素移出xxx_wait指引的队列
		set_current_state(TASK_RUNNING);//设置进程状态为TASK_RUNNING
		return ret;

}




//example 支持阻塞操作的globalfifo 设备驱动
struct globalfifo_dev {
	struct cdev cdev;
	unsigned int current_len;//目前FIFO中有效数据的长度
	unsigned char mem[GLOBALFIFO_SIZE];
	struct mutex mutex;
	wait_queue_head_t r_wait;  //需要在模块加载函数中初始化
	wait_queue_head_t w_wair;  //需要在模块加载函数中初始化
};


static int __init globalfifo_init(void)
{
	int ret;
	dev_t devno = MKDEV(globalfifo_major, 0);

	if(globalfifo_major){
		ret = register_chrdev_region(devno, 1, "globalfifo");
	}else{
		ret = alloc_chrdev_region(&devno, 0, 1, "globalfifo");
		globalfifo_major = MAJOR(devno);
	}

	if(ret < 0){
		return ret;
	}

	globalfifo_devp = kzalloc(sizeof(struct globalfifo_dev), GFP_KERNEL);

	if(!globalfifo_devp){
		ret = -ENOMEM;
		goto fail_malloc;
	}

	globalfifo_setup_cdev(globalfifo_devp, 0);

	mutex_init(&globalfifo_devp->mutex);

	init_waitqueue_head(&globalfifo_devp->r_wait);
	init_waitqueue_head(&globalfifo_devp->w_wait);

	return 0;

fail_malloc:
	unregister_chrdev_region(devno, 1);
	return ret;

}
module_init(globalfifo_init);



static ssize_t globalfifo_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	int ret;
	struct globalfifo_dev *dev = filp->private_data;

	DECLARE_WAITQUEUE(wait, current);

	mutex_lock(&dev->mutex);
	add_wait_queue(&dev->r_wait, &wait);

	while(dev->current_len == 0){
		if(filp->f_flags & O_NONBLOCK){
			ret = -EAGAIN;
			goto out;
		}
		__set_current_state(TASK_INTERRUPTIBLE);//切换进程状态
		mutex_unlock(&dev->mutex);

		schedule();					//显示通过schedule()调度其他进程执行
		if(signal_pending(current)){//醒来的是有需注意，由于调度出去的时候，进程状态是
									//TASK_INTERRUPTIBLE浅睡眠，所以唤醒它的有可能是信号，
									//因此，我们首先通过signal_pending(current)了解是不是信号唤醒的
									//如果是，立即返回 -ERESTARTSYS.
			ret = -ERESTARTSYS;
			goto our2;
		}

		mutex_lock(&dev->mutex);
	}
	
	if(count > dev->current_len){
		count = dev->current_len;
	}

	if(copy_to_user(buf, dev->mem, count)){
		ret = -EFAULT;
		goto out;
	}else{
		memcpy(dev->mem, dev->mem + count, dev->current_len - count);
		dev->current_len -= count;

		printk(KERN_INFO "read %d bytes(s), current_len:%d\n", count, dev->current_en);

		wake_up_interruptible(&dev->w_wait);

		ret = count;

	}
	
out:
	mutex_unlock(&dev->mutex);
out2:
	remove_wait_queue(&dev->w_wait, &wait);

	set_current_state(TASK_RUNNING);

	return ret;


}


static ssize_t globalfifo_write(struct file *filp, const __user *buf,
								size_t count, loff_t *ppos)
{
	struct globalfifo_dev *dev = filp->private_data;
	int ret;
	
	DECLARE_WAITQUEUE(wait, current);
	
	mutex_lock(&dev->w_wait, &wait);
	add_wait_queue(&dev->w_wait, &wait);
	
	while(dev->current_len == GLOBALFIFO_SIZE){
		if(filp->f_flags & O_NONBLOCK){
			ret = -EAGAIN;
			goto out;
		}	
		__set_current_state(TASK_INTERRUPTIBLE);
		
		mutex_unlock(&dev->mutex);
		
		schedule();
		if(signal_pending(current)){
			ret = -ERESTATSYS;
			goto out2;
		}
		
		mutex_lock(&dev->mutex);
	}
	
	if(count > GLOBALFIFO_SIZE - dev->current_len);
		count = GLOBALFIFO_SIZE - dev->current_len;
		
	if(copy_form_user(dev->mem + dev->current_len, buf, count)){
		ret = -EFAULT;
		goto out;
	}else{
		dev->current_len += count;
		printk(KERN_INF "write %d bytes(s),current_len:%d\n",count,dev->current_len);
		
		wake_up_interruptible(&dev->r_wait);
		
		ret = count;
	}
out:
	mutex_unlock(&dev->mutex);
out2:
	remove_wait_queue(&dev->w_wait, &wait);
	set_current_state(TASK_RUNNING);
	return ret;
}

mknode	/dev/globalfifo	c 231 0





































































































































