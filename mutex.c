//互斥体
//1.定义互斥体
struct mutex  my_mutex;
mutex_init(&my_mutex);

//获取互斥体
void mutex_lock(struct mutex *lock);  //引起的睡眠不能被信号打断
int mutex_lock_interruptible(struct mutex *lock);//引起的睡眠可以被打断
int mutex_trylock(struct mutex *lock);//不会引起睡眠，可用于中断上下文


//释放互斥体
void mutex_unlock(struct mutex *lock);

//互斥体使用方法
struct mutex my_mutex;		//定义mutex
mutex_init(&my_mutex);		//初始化mutex

mutex_lock(&my_mutex);		//获取mutex
......						//临界资源

mutex_unlock(&my_mutex);	//释放mutex

//总结：
/*
自旋锁和互斥体都是解决互斥问题的基本手段，面对特定的情况，应该如何取舍这两种手段呢？
选择的依据是临界区的性质和系统的特点。


从严格意义上说，互斥体和自旋锁属于不同层次的互斥手段，互斥体依赖于自旋锁。

自旋锁更底层

互斥体属于进程级别的，用于多个进程之间对资源的互斥，虽然也在内核中，但是该内核执行路径是
以进程的身份，代表进程来争夺资源的。如果竞争失败，会发生进程上下文切换，当前进程进入
睡眠状态，CPU将运行其他进程。
鉴于进程上下文切换的开销也很大，因此，只有当进程占用资源时间比较长时，用互斥体才是
最好的选择。

当要保护的临界区访问时间比较短时，用自旋锁 非常sh方便的，因为它可节省上下文切换的时间。




总结出 自旋锁和互斥体选用的3个原则
1）临界区比较小：自旋锁，灵界去比较大：互斥体
2）保护的临界区是否包含可能引起阻塞的代码，包含则用：互斥体，不包含则用：自旋锁
3）如果被保护的共享资源需要在中断或软中断情况下使用，互斥体和自旋锁只能选择自旋锁，
   非要选择互斥体只能通过mutex_trylock()方式，避免阻塞
   
*/










































































































































































