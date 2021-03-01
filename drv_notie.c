//整型原子操作

void atomic_set(atomic_t *v, int i);/*设置原子变量的值为 i*/

atomic_t v = ATOMIC_INIT(0);/*定义原子变量 v 并初始化为 0*/

atomic_read(atomic_t *v) //返回原子变量的值

void atomic_add(int i, atomic_t *v);//原子变量增加 i

void atomic_sub(int i, atomic_t *v);//原子变量减少 i

void atomic_inc(atomic_t *v);//原子变量增加 1

void atomic_dec(atomic_t *v);//原子变量减少 1 

int atomic_inc_and_test(atomic_t *v);//操作并测试

int atomic_dec_and_test(atomic_t *v);//操作并测试

int atomic_sub_and_test(int i, atomic_t *v);//操作并测试


int atomic_add_return(int i, atomic_t *v);//操作并返回
int atomic_sub_return(int i, atomic_t *v);
int atomic_inc_return(atomic_t *v);
int atomic_dec_return(atomic_t *v);

//位原子操作
void set_bit(nr, void *addr);//设置位
void clear_bit(nr, void *addr);//清除位
void change_bit(nr, void *addr);//改变位
test_bit(nr, void *addr);//测试位
int test_and_set_bit(nr, void *addr);//测试并操作位
int test_and_clear_bit(nr, void *addr);
int test_and_change_bit(nr, void *addr);



//example
static atomic_t xxx_available = ATOMIC_INIT(1);//定义原子变量

static int xxx_open(struct inode *inode, struct file *filp)
{
	...
	if(!atomic_dec_and_test(&xxx_available)){
		atomic_inc(&xxx_available);//已经打开
		return -EBUSY;
	}
	...
	return 0;//打开成功
}


static int xxx_release(struct inode *inode, struct file *filp){
	atomic_inc(&xxx_available);//释放设备
	return 0;
}





































