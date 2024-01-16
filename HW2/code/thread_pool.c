#include "threadpool.h"


Jobnode Pop(threadpool* pool)//pop相应的任务节点
{
	/*******************************************************************/
	sem_wait(&pool->sem);//上锁
	if(pool->jobnum==0){//任务为0，做特殊处理，线程不需要执行任务
	sem_post(&pool->sem);
	return pool->poolhead->data;//data为NULL
	}
	pool->jobnum--;
	threadjob*head=pool->poolhead;
	pool->poolhead=head->next;
	head->next=NULL;
	sem_post(&pool->sem);//解锁
	return head->data;
    	//PLEASE ADD YOURs CODES

   	/******************************************************************/
}


void Job_running(threadpool* pool)// 线程池中的每个线程执行此函数
{	
	while(pool->flag==1){//pool未回收则一直循环执行任务
	while(pool->jobnum==0);//无任务时忙等
	Jobnode job = Pop(pool);  // pop函数里面会进行互斥和等待非空 
	if(job.pf!=NULL){
	job.pf(job.arg);  // 执行任务函数
	}	
	}
	pthread_exit(0);
}


threadpool* Pool_init(int Maxthread)//构造函数
{
	
	threadpool* pool;
	pool = (threadpool*)malloc(sizeof(threadpool));

	pool->flag = 1;//flag=0时表示被回收
	
	/*******************************************************************/
	pool->threads=(pthread_t*)malloc(sizeof(pthread_t)*Maxthread);
    	//PLEASE ADD YOURs CODES
	if(pool->threads==NULL){//分配空间失败
		printf("malloc threads fail\n");
		return NULL;
	}
	//初始化
	pool->poolhead=(threadjob*)malloc(sizeof(threadjob));
	pool->Maxthread=Maxthread;
	for(int i=0;i<pool->Maxthread;i++)pthread_create(&pool->threads[i],NULL,(void*)Job_running,pool);//从线程池取出线程执行任务函数
	pool->poolhead->next=NULL;
	pool->poolhead->data.pf=NULL;
	pool->jobnum=0;
	
	sem_init(&pool->sem, 0, 1);//每次只能一个线程加入线程池操作
   	/******************************************************************/
	
	return pool;
}

// 主线程添加任务到线程池中
int Add_job(threadpool* pool , function_t pf , void* arg)
{
	/*******************************************************************/
	sem_wait(&pool->sem);//上锁
	//添加任务
    threadjob *head =pool->poolhead;
	while(head->next!=NULL){
		head=head->next;
	}
	head->data.pf=pf;
	head->data.arg=arg;
	pool->jobnum++;
	threadjob*new_node=(threadjob*)malloc(sizeof(threadjob));
	head->next=new_node;
	new_node->next=NULL;
    sem_post(&pool->sem);//解锁

    //PLEASE ADD YOURs CODES
	return 1;
   	/******************************************************************/
}

int Push(threadpool* pool , Jobnode data )//push相应的任务节点
{
	/*******************************************************************/
	sem_wait(&pool->sem);
	//添加任务
    threadjob *head =pool->poolhead;
	while(head->next!=NULL){
		head=head->next;
	}
	head->data.pf=data.pf;
	head->data.arg=data.arg;
	pool->jobnum++;
	threadjob*new_node=(threadjob*)malloc(sizeof(threadjob));
	head->next=new_node;
	new_node->next=NULL;
    sem_post(&pool->sem);

    	//PLEASE ADD YOURs CODES
	return 1;
   	/******************************************************************/
}


int Delete_pool(threadpool* pool)//删除线程池
{
	
	pool->flag = 0;
	/*******************************************************************/
	
	free(pool->threads);
	free(pool);
	return 0;
    	//PLEASE ADD YOURs CODES
   	/******************************************************************/
	
}
