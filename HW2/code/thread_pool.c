#include "threadpool.h"


Jobnode Pop(threadpool* pool)//pop��Ӧ������ڵ�
{
	/*******************************************************************/
	sem_wait(&pool->sem);//����
	if(pool->jobnum==0){//����Ϊ0�������⴦���̲߳���Ҫִ������
	sem_post(&pool->sem);
	return pool->poolhead->data;//dataΪNULL
	}
	pool->jobnum--;
	threadjob*head=pool->poolhead;
	pool->poolhead=head->next;
	head->next=NULL;
	sem_post(&pool->sem);//����
	return head->data;
    	//PLEASE ADD YOURs CODES

   	/******************************************************************/
}


void Job_running(threadpool* pool)// �̳߳��е�ÿ���߳�ִ�д˺���
{	
	while(pool->flag==1){//poolδ������һֱѭ��ִ������
	while(pool->jobnum==0);//������ʱæ��
	Jobnode job = Pop(pool);  // pop�����������л���͵ȴ��ǿ� 
	if(job.pf!=NULL){
	job.pf(job.arg);  // ִ��������
	}	
	}
	pthread_exit(0);
}


threadpool* Pool_init(int Maxthread)//���캯��
{
	
	threadpool* pool;
	pool = (threadpool*)malloc(sizeof(threadpool));

	pool->flag = 1;//flag=0ʱ��ʾ������
	
	/*******************************************************************/
	pool->threads=(pthread_t*)malloc(sizeof(pthread_t)*Maxthread);
    	//PLEASE ADD YOURs CODES
	if(pool->threads==NULL){//����ռ�ʧ��
		printf("malloc threads fail\n");
		return NULL;
	}
	//��ʼ��
	pool->poolhead=(threadjob*)malloc(sizeof(threadjob));
	pool->Maxthread=Maxthread;
	for(int i=0;i<pool->Maxthread;i++)pthread_create(&pool->threads[i],NULL,(void*)Job_running,pool);//���̳߳�ȡ���߳�ִ��������
	pool->poolhead->next=NULL;
	pool->poolhead->data.pf=NULL;
	pool->jobnum=0;
	
	sem_init(&pool->sem, 0, 1);//ÿ��ֻ��һ���̼߳����̳߳ز���
   	/******************************************************************/
	
	return pool;
}

// ���߳���������̳߳���
int Add_job(threadpool* pool , function_t pf , void* arg)
{
	/*******************************************************************/
	sem_wait(&pool->sem);//����
	//�������
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
    sem_post(&pool->sem);//����

    //PLEASE ADD YOURs CODES
	return 1;
   	/******************************************************************/
}

int Push(threadpool* pool , Jobnode data )//push��Ӧ������ڵ�
{
	/*******************************************************************/
	sem_wait(&pool->sem);
	//�������
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


int Delete_pool(threadpool* pool)//ɾ���̳߳�
{
	
	pool->flag = 0;
	/*******************************************************************/
	
	free(pool->threads);
	free(pool);
	return 0;
    	//PLEASE ADD YOURs CODES
   	/******************************************************************/
	
}
