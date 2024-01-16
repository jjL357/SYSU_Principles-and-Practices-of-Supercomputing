#include "threadpool.h"

void task1(void)
{
	long sum = 0, i;
	for( i=0; i<1000; i++ ){
		sum = ( sum + rand() ) % INT_MAX;
	}
	printf("this is the task1 running in <%ld>, the answer = %ld\n", gettid(), sum );
}

void task2(void)
{
	long sum = 0, i;
	for(i=0; i<1000; i++){
		sum = ( sum + i * rand() ) % INT_MAX;
	}
	printf("this is the task2 running in <%ld>, the answer = %ld\n", gettid(), sum );
}

int main()
{
	// ��ʼ��һ���̳߳�
	threadpool* pool = Pool_init(10);

	// ���߳������Ӧ�������̳߳���
	int i=0;
	for(; i<100; i++){
		Add_job(pool ,(void*)task1 , NULL );
		Add_job(pool , (void*)task2 , NULL);
	}

	// �ȴ����������
	sleep(10);

	// ɾ���̳߳�
    Delete_pool(pool);

	return 0;
}