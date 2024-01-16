#ifndef __POOL__
#define __POOL__

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <string.h>
#include <sys/syscall.h>
#include <semaphore.h>
#include <limits.h>
typedef void* (*function_t)(void* arg);
#define gettid() syscall(__NR_gettid)

// ����ڵ㣬ÿ�������ú���ָ��Ͷ�Ӧ�Ĳ�����ʾ
// ִ������ʱ�� pf(arg) ���ɣ�pf��ʾ��Ӧ�ĺ�����argΪ��Ӧ�Ĵ������
typedef struct Jobnode{
	function_t pf;
	void* arg;
}Jobnode;

// ������У���һ�����б�ʾ
typedef struct threadjob{
	Jobnode data;
	struct threadjob* next;
}threadjob;

// �̳߳�
typedef struct threadpool{
	threadjob* poolhead; //������е�ͷ�ڵ�
	int jobnum;			 // ��������е�����ڵ���Ŀ
	int Maxthread;		// �̳߳��п����ɵ�����߳���Ŀ
	pthread_t* threads; // �̳߳���ά�����߳̾��
	sem_t sem;			// �̳߳��е�ͬ�������������ź����򻥳���
	int flag;           // �̳߳ؽ�����־��Ϊ0ʱ�������й�����߳�
	pthread_t cur_thread;				// ������ʵ�ֹ����п�����Ҫ�Ĳ������������
}threadpool;

//operation of threadpool

//���캯��
threadpool* Pool_init(int Maxthread);				

//ɾ���̳߳�
// ���մ����̳߳������ٵĿռ䣬ע��Ҫ������������еĿռ�
int Delete_pool(threadpool* pool);					

// ���߳���������̳߳���
int Add_job(threadpool* pool , function_t pf , void* arg); 

// �̳߳��е�ÿ���߳�ִ�д˺���
// ���������зǿգ�ȡ��һ������ڵ�ִ��
// ����������Ϊ�գ�����ȴ�
// �漰����̴߳����������ȡ����ڵ㣬ע�⿼��ͬ��������
void Job_running(threadpool* pool);


// ������еĲ�����push, pop��Ӧ������ڵ�
// ����ֻ�Ǹ�һ��ʾ����ʵ��ʵ�ֿ��Խ�����Ӧ�ĸ���
Jobnode Pop(threadpool* pool);

int Push(threadpool* pool , Jobnode data );

//

#endif