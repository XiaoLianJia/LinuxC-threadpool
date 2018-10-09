#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/signal.h>
#include "threadpool.h"


////////////////////////////////////////////////////////////
// ���ܣ������̳߳�
// ���룺�̳߳����̸߳�����Сֵ���̳߳����̸߳������ֵ������������ֵ
// �����
// ���أ��̳߳ص�ַ
////////////////////////////////////////////////////////////
THREAD_POOL* thread_pool_create(int thread_count_min, int thread_count_max, int task_count_max)
{
	// Ϊ�̳߳ؿ����ڴ�ռ�
	THREAD_POOL* thread_pool = NULL;
	thread_pool = (THREAD_POOL*)malloc(sizeof(THREAD_POOL));
	if (NULL == thread_pool) return NULL;

	// ��ʼ���̳߳ز���
	thread_pool->shutdown = 0;
	thread_pool->thread_count_min = thread_count_min;
	thread_pool->thread_count_max = thread_count_max;
	thread_pool->thread_count_live = thread_count_min;
	thread_pool->thread_count_busy = 0;
	thread_pool->thread_count_over = 0;

	thread_pool->task_count_max = task_count_max;
	thread_pool->task_count_wait = 0;
	thread_pool->task_head = NULL;
	thread_pool->task_tail = NULL;

	// Ϊ�����߳̿����ڴ�ռ�
	thread_pool->pthreads = (pthread_t*)malloc(sizeof(pthread_t) * thread_count_max);
	if (NULL == thread_pool->pthreads) {
		thread_pool_destroy(thread_pool);
		return NULL;
	}

	// ��ʼ������������������
	if (pthread_mutex_init(&(thread_pool->lock), NULL) 
		|| pthread_mutex_init(&(thread_pool->counter), NULL)
		|| pthread_cond_init(&(thread_pool->queue_not_empty), NULL) 
		|| pthread_cond_init(&(thread_pool->queue_not_full), NULL)) {

		thread_pool_destroy(thread_pool);
		return NULL;
	}

	// ���������߳�
	int i;
	for (i = 0; i < thread_pool->thread_count_min; i++) {
		pthread_create(&(thread_pool->pthreads[i]), NULL, thread_pool_work, (void*)thread_pool);
	}

	// ���������߳�
	pthread_create(&(thread_pool->admin), NULL, thread_pool_admin, (void*)thread_pool);
	return thread_pool;
}

////////////////////////////////////////////////////////////
// ���ܣ������̳߳�
// ���룺�̳߳ص�ַ
// �����
// ���أ���
////////////////////////////////////////////////////////////
void thread_pool_destroy(THREAD_POOL* thread_pool)
{
	if (NULL == thread_pool || thread_pool->shutdown) return;

	// �̳߳�����
	pthread_mutex_lock(&(thread_pool->lock));

	// ����ִ������
	thread_pool->task_count_wait = 0;
	thread_pool->shutdown = 1;

	// ɱ�����й����߳�
	int i;
	for (i = 0; i < thread_pool->thread_count_live; i++) {
		pthread_cond_signal(&(thread_pool->queue_not_empty));
	}

	// �̳߳ؽ���
	pthread_mutex_unlock(&(thread_pool->lock));

	// �ͷ���Դ	
	free(thread_pool->pthreads);
	pthread_mutex_destroy(&(thread_pool->lock));
	pthread_cond_destroy(&(thread_pool->queue_not_empty));
	pthread_cond_destroy(&(thread_pool->queue_not_full));

	THREAD_TASK* p;
	while (thread_pool->task_head != NULL) {
		p = thread_pool->task_head;
		thread_pool->task_head = p->next;
		free(p);
	}
	free(thread_pool);
}

////////////////////////////////////////////////////////////
// ���ܣ��������
// ���룺�̳߳ص�ַ���ص��������ص���������
// �����
// ���أ�0-�ɹ� -1-ʧ��
////////////////////////////////////////////////////////////
int thread_pool_add_task(THREAD_POOL* thread_pool, void* (*callback_function)(void *arg), void *arg)
{
	if (NULL == thread_pool || NULL == callback_function || NULL == arg) return -1;

	// �̳߳�����
	pthread_mutex_lock(&(thread_pool->lock));

	// �ȴ��������ӯ��
	while ((thread_pool->task_count_wait == thread_pool->task_count_max) && !thread_pool->shutdown) {
		pthread_cond_wait(&(thread_pool->queue_not_full), &(thread_pool->lock));
	}

	// �̳߳�������
	if (thread_pool->shutdown) {
		pthread_mutex_unlock(&(thread_pool->lock));
		return -1;
	}

	// Ϊ���񿪱��ڴ�ռ�
	THREAD_TASK* task = (THREAD_TASK*)malloc(sizeof(THREAD_TASK));
	if (NULL == task) {
		pthread_mutex_unlock(&(thread_pool->lock));
		return -1;
	}

	// ��ʼ������
	task->callback_function = callback_function;
	task->arg = arg;
	task->next = NULL;

	// ��ӵ��������
	thread_pool->task_count_wait++;
	if (thread_pool->task_head == NULL) {
		thread_pool->task_head = thread_pool->task_tail = task;
	}
	else {
		thread_pool->task_tail->next = task;
		thread_pool->task_tail = task;
	}

	// ������зǿ�
	pthread_cond_broadcast(&(thread_pool->queue_not_empty));

	// �̳߳ؽ���
	pthread_mutex_unlock(&(thread_pool->lock));
	return 0;
}

////////////////////////////////////////////////////////////
// ���ܣ������߳�
// ���룺�̳߳ص�ַ
// �����
// ���أ���
////////////////////////////////////////////////////////////
void* thread_pool_admin(void* arg)
{
	THREAD_POOL* thread_pool = (THREAD_POOL*)arg;

	while (!thread_pool->shutdown) {

		sleep(10);

		// �̳߳�����
		pthread_mutex_lock(&(thread_pool->lock));


		//printf("============= Admin Start ============\n");
		//printf("thread_count_min: --%d--\n", thread_pool->thread_count_min);
		//printf("thread_count_max: --%d--\n", thread_pool->thread_count_max);
		//printf("thread_count_live: --%d--\n", thread_pool->thread_count_live);
		//printf("thread_count_busy: --%d--\n", thread_pool->thread_count_busy);
		//printf("thread_count_over: --%d--\n", thread_pool->thread_count_over);
		//printf("task_count_max: --%d--\n", thread_pool->task_count_max);
		//printf("task_count_wait: --%d--\n", thread_pool->task_count_wait);


		// ������е�ǰֵ
		int task_count = thread_pool->task_count_wait;
		// �̳߳��л�Ծ�̸߳���
		int thread_count_live = thread_pool->thread_count_live;
		// �̳߳��з�æ�̸߳���
		int thread_count_busy = thread_pool->thread_count_busy;

		// �������߳�
		if (task_count >= thread_pool->task_count_max) {

			int i;
			for (i = 0; i < thread_pool->thread_count_max; i++) {

				if (thread_count_live >= thread_pool->thread_count_max) break;
				if (task_count <= thread_pool->task_count_max / 2) break;

				if (thread_pool->pthreads[i] == 0 || ESRCH == pthread_kill(thread_pool->pthreads[i], 0)) {
					pthread_create(&(thread_pool->pthreads[i]), NULL, thread_pool_work, (void*)thread_pool);
					thread_pool->thread_count_live++;
					task_count--;
				}
			}
		}

		// ���ٹ�ʣ�߳�
		if ((thread_count_busy * 2) < thread_count_live && thread_count_live > thread_pool->thread_count_min) {

			thread_pool->thread_count_over = (thread_count_live - thread_count_busy) / 2;
			int i;
			for (i = 0; i < (thread_count_live - thread_count_busy) / 2; i++) {
				pthread_cond_signal(&(thread_pool->queue_not_empty));
			}
		}


		//printf("============= Admin End ============\n");
		//printf("thread_count_min: --%d--\n", thread_pool->thread_count_min);
		//printf("thread_count_max: --%d--\n", thread_pool->thread_count_max);
		//printf("thread_count_live: --%d--\n", thread_pool->thread_count_live);
		//printf("thread_count_busy: --%d--\n", thread_pool->thread_count_busy);
		//printf("thread_count_over: --%d--\n", thread_pool->thread_count_over);
		//printf("task_count_max: --%d--\n", thread_pool->task_count_max);
		//printf("task_count_wait: --%d--\n", thread_pool->task_count_wait);


		// �̳߳ؽ���
		pthread_mutex_unlock(&(thread_pool->lock));
	}

	return NULL;
}

////////////////////////////////////////////////////////////
// ���ܣ������߳�
// ���룺�̳߳ص�ַ
// �����
// ���أ���
////////////////////////////////////////////////////////////
void* thread_pool_work(void* arg)
{
	THREAD_POOL* thread_pool = (THREAD_POOL*)arg;

	while (1) {
		// �̳߳�����
		pthread_mutex_lock(&(thread_pool->lock));

		// �����ʣ�̣߳��ȴ�����
		while ((thread_pool->task_count_wait == 0) && !thread_pool->shutdown) {
			
			pthread_cond_wait(&(thread_pool->queue_not_empty), &(thread_pool->lock));

			if (thread_pool->thread_count_over > 0) {
				thread_pool->thread_count_over--;
				if (thread_pool->thread_count_live > thread_pool->thread_count_min) {
					thread_pool->thread_count_live--;
					pthread_mutex_unlock(&(thread_pool->lock));				
					pthread_exit(NULL);
				}
			}
		}

		// �̳߳�������
		if (thread_pool->shutdown) {
			pthread_mutex_unlock(&(thread_pool->lock));
			pthread_exit(NULL);
		}

		// �������
		THREAD_TASK* task = NULL;
		task = thread_pool->task_head;
		thread_pool->task_count_wait--;
		thread_pool->thread_count_busy++;

		if (thread_pool->task_count_wait == 0) {
			thread_pool->task_head = thread_pool->task_tail = NULL;
		}
		else {
			thread_pool->task_head = task->next;
		}

		// �������δ��
		if (thread_pool->task_count_wait < thread_pool->task_count_max) {
			pthread_cond_broadcast(&(thread_pool->queue_not_full));
		}

		// �̳߳ؽ���
		pthread_mutex_unlock(&(thread_pool->lock));

		// ִ������
		(*(task->callback_function))(task->arg);
		free(task);
		task = NULL;

		pthread_mutex_lock(&(thread_pool->counter));
		thread_pool->thread_count_busy--;
		pthread_mutex_unlock(&(thread_pool->counter));
	}
}
