//信号量
//1.定义信号量
struct semaphore sem;

//2.初始化信号量
//该函数初始化信号量，并设置信号量sem的值为val
void sema_init(struct semaphore *sem, int val);

//3.获得信号量
//该函数用于获得信号量sem，它会导致睡眠，因此不能在中断上下文中使用
void down(struct semaphore *sem);


//该函数功能与down()类似，不同之处为，进入睡眠状态的进程能被信号打断
int down_interruptible(struct semaphore *sem);
//使用例子：
if(down_interruption(&sem)){
	return -ERRSTARTSYS;
}


//不会导致调用者睡眠，可在中断上下文中使用
int down_trylock(struct semaphore *sem);


//释放信号量
//释放信号量sem，唤醒等待者
void up(struct semaphore *sem);


//说明：
//作为一种可能的互斥手段，信号量可以保护临界区，它的使用方式和自旋锁类似。
//与自旋锁相同，只有得到信号量的进程才能执行临界区代码。
//但与自旋锁不同的是，当获取不到信号量时，进程不会原地打转而是进入睡眠等待状态

//新内核推荐直接使用mutex作为互斥手段，信号量用作互斥不再被推荐。

































































































































































