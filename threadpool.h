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
	int shutdown;								// �̳߳��Ƿ�����
	int thread_count_min;						// �̳߳����̸߳�����Сֵ
	int thread_count_max;						// �̳߳����̸߳������ֵ
	int thread_count_live;						// �̳߳��л�Ծ�̸߳���
	int thread_count_busy;						// �̳߳��з�æ�̸߳���
	int thread_count_over;						// �̳߳��й�ʣ�̸߳���

	int task_count_max;							// ����������ֵ
	int task_count_wait;						// ������е�ǰֵ
	THREAD_TASK* task_head;						// ָ��������е�ͷָ��
	THREAD_TASK* task_tail;						// ָ��������е�βָ��

	pthread_t* pthreads;						// �̳߳��й����߳�����ĵ�ַ
	pthread_t admin;							// �����߳�
	pthread_mutex_t lock;						// �̳߳ػ����ź���
	pthread_mutex_t counter;					// �̼߳��������ź���
	pthread_cond_t queue_not_empty;				// ������в�Ϊ�յ���������
	pthread_cond_t queue_not_full;				// ������в�Ϊ������������

}THREAD_POOL;


// �����̳߳�
THREAD_POOL* thread_pool_create(int thread_count_min, int thread_count_max, int task_count_max);
// �����̳߳�
void thread_pool_destroy(THREAD_POOL* thread_pool);
// �������
int thread_pool_add_task(THREAD_POOL* thread_pool, void* (*callback_function)(void *arg), void *arg);
// �����߳�
void* thread_pool_admin(void* arg);
// �����߳�
void* thread_pool_work(void* arg);

#endif // !THREAD_H
