#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#define NUMS 100 //��ʾ���������ѵĴ���
#define CAPACITY 5 //���建�������ֵ
int capacity = 0; //��ǰ�������Ĳ�Ʒ����
pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;//������

void *produce(void *args)//�����ߺ���
{
     /*******************************************************************/
    int produce_times=3;//�̶������Ĵ�������
    while(produce_times>0){//��δ�ﵽ�̶������Ĵ������ޣ�����������
    if(capacity>=CAPACITY)continue;//��ֹ�����ľ�����ɵļ�������
    pthread_mutex_lock(&mylock);//����
    if(capacity<CAPACITY)produce_times--;//��δ�ﵽ���������ֵ�����������
    while(capacity<CAPACITY){//����ֱ������������
        printf("Producer:capacity=%d\n",++capacity);//�����������Ʒ����
    }
    printf("Producer:The buffer is full!\n");
    pthread_mutex_unlock(&mylock);//����
    }
    
    return NULL;


   /******************************************************************/
}
void * consume(void *args)//�����ߺ���
{
     /*******************************************************************/
    int consume_times=3;//�̶����ѵĴ�������
    while(consume_times>0){//��δ�ﵽ�̶����ѵĴ������ޣ���������
    if(capacity<=0)continue;//��ֹ�����ľ�����ɵ��޷���������
    pthread_mutex_lock(&mylock);//����
    if(capacity>0)consume_times--;//���������ǿգ����������
    while(capacity>0){//����ֱ����������
        printf("Consumer:capacity=%d\n",--capacity);//�����������Ʒ����
    }
    printf("Consumer:The buffer is empty!\n");
    pthread_mutex_unlock(&mylock);//����
    }
    return NULL;
  



   /******************************************************************/
}
int main(int argc, char** argv) {
    int err;
    pthread_t produce_tid, consume_tid;
    void *ret;
    err = pthread_create(&produce_tid, NULL, produce, NULL);//�����߳�
    if (err != 0) {
        printf("�̴߳���ʧ��:%s\n", strerror(err));
        exit(-1);
    }
    err = pthread_create(&consume_tid, NULL, consume, NULL);
    if (err != 0)  {
        printf("�̴߳���ʧ��:%s\n", strerror(err));
        exit(-1);
    }
    err = pthread_join(produce_tid, &ret);//���̵߳ȵ����߳��˳�
    if (err != 0) {
        printf("�������̷ֽ߳�ʧ��:%s\n", strerror(err));
        exit(-1);
    }
    err = pthread_join(consume_tid, &ret);
    if (err != 0) {
        printf("�������̷ֽ߳�ʧ��:%s\n", strerror(err));
        exit(-1);
    }
    
    return (EXIT_SUCCESS);
}