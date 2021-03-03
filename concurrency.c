//增加并发控制后的globalmem的设备驱动

//由于copy_form_user()、copy_to_user()这些可能导致
//阻塞的函数，因此不能使用自旋锁，宜使用互斥体

//驱动工程师习惯上将某设备所使用的自旋锁、互斥体等辅助手段
//也放在设备结构体中


struct globalmem_dev {
	struct cdev cdev;
	unsigned char mem[GLOBALMEM_SIZE];
	struct mutex mutex;
};


static int __init globalmem_init(void)
{
	int ret;
	dev_t devno = MKDEV(globalmem_major, 0);

	if(globalmem_major){
		ret = register_chrdev_region(devno, 1, "globalmem");
	}else{
		ret = alloc_chrdev_region(&devno, 0, 1, "globalmem");
		globalmem_major = MAJOR(devno);
	}

	if(ret < 0)
		return ret;

	globalmem_devp = kzalloc(sizeof(struct globalmem_dev), GFP_KERNEL);

	if(!globalmem_devp){
		ret = -ENOMEM;
		goto fail_malloc;
	}

	mutex_init(&globalmem_devp->mutex);
	globalmem_setup_cdev(globalmem_devp, 0);
	return 0;

fail_malloc:
	unregister_chrdev_region(devno, 1);
	return ret;

}

module_init(globalmem_init);


static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int  count = size;
	int ret = 0;

	struct globalmem_dev *dev = filp->private_data;

	if(p >= GLOBALMEM_SIZE)
		return 0;

	if(count > GLOBALMEM_SIZE - p)
		count  = GLOBALMEM_SIZE - p;


	mutex_lock(&dev->mutex);

	if(copy_to_user(buf, dev->mem + p, count)){
		ret = -EFAULT;
	}else{
		*ppos += count;
		ret = count;

		printk(KERN_INFO "read %u bytes(s) from %lu \n", count ,p);
	}

	mutex_unlock(&dev->mutex);

	return ret;
}



static ssize_t globalmem_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;

	struct globalmem_dev *dev = filp->private_data;

	if(p >= GLOBALMEM_SIZE)
		return 0;

	mutex_lock(&dev->mutex);

	if(copy_from_user(dev->mem + p, buf, count)
		ret = -EFAULLT;
	else{
		*ppos += count;
		ret = count;

		printk(KERN_INFO "written %u bytes(s) from %lu\n",count ,p);
	}

	mutex_unlock(&dev->mutex);

	return ret;
}




static long globalmem_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct globalmem_dev *dev = filp->private_data;

	switch(cmd){
		case MEM_CLEAR:
			mutex_lock(&dev->mutex);
			memset(dev->mem, 0, GLOBALMEM_SIZE);
			mutex_unlock(&dev->mutex);

			printk(KERN_INFO "globalmem is set to zero\n");
			break;

		default:
			return -EINVAL;	
	}
	
		return 0;
}
	


/*

总结：
并发和竞态广泛存在，中断屏蔽、 原子操作、自旋锁、互斥体都是解决并发问题的机制。
1.中断屏蔽很少被单独使用。
2.原子操作只能针对整数进行
3.因此自旋锁和互斥体应用最为广泛
自旋锁会导致死循环，锁定期间不允许阻塞，因此要求锁定的临界区小。
互斥体允许临界区阻塞，可适用于临界区大的情况。


*/




































































































































































































