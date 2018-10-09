#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>


////////////////////////////////////////////////////////////
// �����������ݽṹ
////////////////////////////////////////////////////////////
typedef struct _THREAD_TASK
{
	void* (*callback_function)(void* arg);		// �̻߳ص�����
	void* arg;									// �ص���������
	struct _THREAD_TASK* next;

}THREAD_TASK;

////////////////////////////////////////////////////////////
// �̳߳����ݽṹ
////////////////////////////////////////////////////////////
typedef struct _THREAD_POOL
{
	int thread_count;							// �̳߳��п����̵߳ĸ���
	int queue_task_max;							// ���������������
	int queue_task_cur;							// ���е�ǰ���������
	THREAD_TASK* task_head;						// ָ��������е�ͷָ��
	THREAD_TASK* task_tail;						// ָ��������е�βָ��
	pthread_t* pthreads;						// �̳߳��������̵߳�pthread_t
	pthread_mutex_t lock;						// �����ź���
	pthread_cond_t queue_empty;					// ����Ϊ�յ���������
	pthread_cond_t queue_not_empty;				// ���в�Ϊ�յ���������
	pthread_cond_t queue_not_full;				// ���в�Ϊ������������
	int queue_close;							// �����Ƿ��Ѿ��ر�
	int pool_close;								// �̳߳��Ƿ��Ѿ��ر�

}THREAD_POOL;


// �����̳߳�
THREAD_POOL* thread_pool_create(int thread_count, int queue_task_max);
// �����̳߳�
void thread_pool_destroy(THREAD_POOL* thread_pool);
// �������
int thread_pool_add_task(THREAD_POOL* thread_pool, void* (*callback_function)(void *arg), void *arg);
// ִ������
void* thread_pool_work(void* arg);


#endif // !THREAD_H
