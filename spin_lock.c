/*---------------------- 自旋锁 -----------------*/
//1定义自旋锁
spinlock_t lock;

//2初始化自旋锁
spin_lock_init(lock);

//3获取自旋锁
spin_lock(lock);

//4释放自旋锁
spin_unlock(lock);



//example
spinlock_t    lock;			//定义自旋锁
spin_lock_init(&lock);
spin_lock(&lock);			//获取自旋锁，保护临界区
	....					//临界区代码
spin_unlock(&lock);			//释放自旋锁


spin_lock_irq() = spin_lock() + local_irq_disable()
spin_unlock_irq() = spin_unlock() + local_irq_enable()
spin_lock_irqsave() = spin_lock() + local_irq_save()
spin_unlock_irqrestore() = spin_unlock() + local_irq_restore()
spin_lock_bh() = spin_lock() + local_bh_disable()
spin_unlock_bh() = spin_unlock() + local_bh_enable()



//example 1 使用自旋锁使设备只能被一个进程打开
int xxx_count = 0; // 定义文件打开次数计数

static int xxx_open(struct inode *inode, struct file *filp)
{
	.....
	spin_lock(&xxx_lock);
	if(xxx_count){				//已经打开
		spin_unlock(&xxx_lock);
		return -EBUSY;
	}

	xxx_count++; //增加使用计数
	spin_unlock(&xxx_lock);
	.....
	return 0;		//成功打开
	
}

static int xxx_release(struct inode *indoe, struct file *filp)
{
	....
	spin_lock(&xxx_lock);
	xxx_count--;
	spin_unlock(&xxx_lock);

	return 0;
}



/*---------------------  读写自旋锁      -------------------------*/
//1.定义和初始化读写自旋锁
rwlock_t my_rwlock;
rwlock_init(&my_rwlock);//动态初始化

//2.读锁定
void read_lock(rwlock_t *lock);
void read_lock_irqsave(rwlock_t *lock, unsigned long flags);
void read_lock_irq(rwlock_t *lock);
void read_lock_bh(rwlock_t *lock);

//3.读解锁
void read_unlock(rwlock_t *lock);
void read_unlock_irqrestore(rwlock_t *lock, unsigned long flags);
void read_unlock_irq(rwlock_t *lock);
void read_unlock_bh(rwlock_t *lock);


//说明：
/*
	read_lock_irqsave()		|read_lock_irq()	|	read_lock_bh()分别是下面两个函数的组合：
	read_lock();		 	|read_lock();		|	read_lock();
	lock_irq_save();	 	|lock_irq_disable();|	local_bh_disable();
*/

//4.写锁定
void write_lock(rwlock_t *lock);
void write_lock_irqsave(rwlock_t *lock, unsigned long flags);
void write_lock_irq(rwlock_t *lock);
void write_lock_bh(rwlock_t *lock);
int write_trylock(rwlock_t *lock);

//5.写解锁
void write_unlock(rwlock_t *lock);
void write_unlock_irqrestore(rwlock_t *lock, unsigned long flags);
void write_unlock_irq(rwlock_t *lock);
void write_unlock_bh(rwlock_t *lock);

//在对共享资源进行写之前，应该先调用写锁定函数，完成之后应调用写解锁函数。和spin_trylock()一样，
//write_trylock()也只是尝试获取读写自旋锁，不管成功失败，都会立即返回。


//example 读写自旋锁一般这样被使用：
rwlock_t lock;//定义rwlock
rwlock_init(&lock);//初始化rwlock

read_lock(&lock);//读时获取锁
.....				  //临界资源
read_unlock(&lock);	  //释放读锁定

write_lock_irqsave(&lock, flags);//写时获取锁
......					//临界资源
write_unlock_irqrestore(&lock, flags);


/*---------------------- 顺序锁 -----------------*/
/*
	顺序锁是对读写锁的一种优化  seqlock
*/

//1.获得顺序锁
void write_seqlock(seqlock_t *sl);
int write_tryseqlock(seqlock_t *sl);
write_seqlock_irqsave(lock, flags); = local_irq_save() + write_seqlock();
write_seqlock_irq(lock);	= local_irq_disable() + write_seqlock();
write_seqlock_bh(lock); = local_bh_disable() +write_seqlock();

//2.释放顺序锁
void write_sequnlock(seqlock_t *sl);
write_sequnlock_irqrestore(lock, flags); = local_irq_restore() + write_sequnlock();
write_sequnlock_irq(lock); = local_irq_enable() + write_sequnlock();
wriet_sequnlock_bh(lock); = local_bh_enable() +write_sequnlock();

//example 写执行单元使用顺序锁的模式如下：
write_seqlock(&seqlock_a);
......//写操作代码块
write_sequnlock(&seqlock_a);
//因此对写执行单元而言，它的使用与自旋锁相同

//example 读执行单元涉及的顺序锁操作如下
//1.读开始
unsigned read_seqbegin(const seqlock_t *sl);
read_seqbegin_irqsave(lock, flags); = local_irq_save() + read_seqbegin();

//2.重读
int read_seqretry(const seqlock_t *sl, unsigned iv);
read_seqretry_irqrestrore(lock, iv, flags); = local_irq_restore() + read_seqretry();

//example 读执行单元使用顺序锁的模式如下：
do{
	seqnum = read_seqbegin(&seqlock_a);
	/* 读操作代码块 */
	........
	
}while(read_seqretry(&seqlock_a, seqnum));













































































































































//5.写解锁















