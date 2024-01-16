#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#define NUMS 100 //表示生产，消费的次数
#define CAPACITY 5 //定义缓冲区最大值
int capacity = 0; //当前缓冲区的产品个数
pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;//互斥量

void *produce(void *args)//生产者函数
{
     /*******************************************************************/
    int produce_times=3;//固定生产的次数上限
    while(produce_times>0){//当未达到固定生产的次数上限，继续生产、
    if(capacity>=CAPACITY)continue;//防止对锁的竞争造成的饥饿问题
    pthread_mutex_lock(&mylock);//上锁
    if(capacity<CAPACITY)produce_times--;//若未达到缓冲区最大值，则可以生产
    while(capacity<CAPACITY){//生产直到不能再生产
        printf("Producer:capacity=%d\n",++capacity);//输出缓冲区产品个数
    }
    printf("Producer:The buffer is full!\n");
    pthread_mutex_unlock(&mylock);//解锁
    }
    
    return NULL;


   /******************************************************************/
}
void * consume(void *args)//消费者函数
{
     /*******************************************************************/
    int consume_times=3;//固定消费的次数上限
    while(consume_times>0){//当未达到固定消费的次数上限，继续消费
    if(capacity<=0)continue;//防止对锁的竞争造成的无法生产问题
    pthread_mutex_lock(&mylock);//上锁
    if(capacity>0)consume_times--;//若缓冲区非空，则可以消费
    while(capacity>0){//消费直到缓冲区空
        printf("Consumer:capacity=%d\n",--capacity);//输出缓冲区产品个数
    }
    printf("Consumer:The buffer is empty!\n");
    pthread_mutex_unlock(&mylock);//解锁
    }
    return NULL;
  



   /******************************************************************/
}
int main(int argc, char** argv) {
    int err;
    pthread_t produce_tid, consume_tid;
    void *ret;
    err = pthread_create(&produce_tid, NULL, produce, NULL);//创建线程
    if (err != 0) {
        printf("线程创建失败:%s\n", strerror(err));
        exit(-1);
    }
    err = pthread_create(&consume_tid, NULL, consume, NULL);
    if (err != 0)  {
        printf("线程创建失败:%s\n", strerror(err));
        exit(-1);
    }
    err = pthread_join(produce_tid, &ret);//主线程等到子线程退出
    if (err != 0) {
        printf("生产着线程分解失败:%s\n", strerror(err));
        exit(-1);
    }
    err = pthread_join(consume_tid, &ret);
    if (err != 0) {
        printf("消费者线程分解失败:%s\n", strerror(err));
        exit(-1);
    }
    
    return (EXIT_SUCCESS);
}