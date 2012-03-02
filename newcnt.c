/*
 * this file is testing how the semaphore used in the macOS
 * several functions: sem_open, sem_wait, sem_post, sem_unlink, sem_close
 */
#include "http.h"

#define DEBUG

#define NITER 1000000
#ifdef DEBUG
#define FILE_NAME "~/srg"
#else
#define FILE_NAME "/tmp/mutex"
#endif
unsigned int cnt = 0;
sem_t *mutex;

void P(sem_t *sem);
void V(sem_t *sem);
void *thread(void *vargp);

int main(void)
{
	pthread_t tid1, tid2;
	int i;

	mutex = sem_open(FILE_NAME, O_RDWR|O_CREAT, 0644, 1);
	if (mutex == SEM_FAILED)
	{
		printf("sem_open failed! (%s)\n",strerror(errno));
		return -1;
	}
	printf("sem_open success!\n");
	pthread_create(&tid1, NULL, thread, NULL);
	pthread_create(&tid2, NULL, thread, NULL);
	pthread_join(tid1, NULL);
	pthread_join(tid2, NULL);

	if (cnt != (unsigned)NITER*2)
		printf("BOOM! cnt=%d\n", cnt);
	else
		printf("OK cnt=%d\n", cnt);
	sem_unlink(FILE_NAME);
	sem_close(mutex);
	exit(0);
}

void *thread(void *vargp)
{
	int i;
	for (i=0; i<NITER; i++) {
		P(mutex);
		cnt++;
		V(mutex);
	}
	return NULL;
}

void P(sem_t *sem)
{
	if (sem_wait(sem) < 0)
		printf("P error!\n");
}

void V(sem_t *sem)
{
	if (sem_post(sem) < 0)
		//printf("V error!\n");
		;
}
