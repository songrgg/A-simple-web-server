#include "http.h"
/*
 * this source file is about sharing resources with semaphore
 */

/*
 * sbuf_init(sbuf_t *, int)
 * initialize the buffer using semaphore to keep synchronized
 * return value: void
 */
void sbuf_init(sbuf_t *sp, int size)
{
	sp->size = size;
	sp->buf = calloc(size, sizeof(int));
	sp->front = sp->rear = 0;
	sp->mutex = sem_open("/tmp/mutex", O_RDWR|O_CREAT, 0644, 1);
	sp->slots = sem_open("/tmp/slots", O_RDWR|O_CREAT, 0644, size);
	sp->items = sem_open("/tmp/items", O_RDWR|O_CREAT, 0644, 0);
}

/*
 * sbuf_insert(sbuf_t *, int)
 * insert an item into the buffer
 * if the buffer is full, then block.
 * return value: void
 */
void sbuf_insert(sbuf_t *sp, int item)
{	
	sem_wait(sp->slots);
	sem_wait(sp->mutex);
	sp->buf[(++sp->rear)%(sp->size)] = item;
	sem_post(sp->mutex);
	sem_post(sp->items);
}

/*
 * sbuf_remove(sbuf_t *)
 * remove an item from the buffer
 * return value: a value of int
 */
int sbuf_remove(sbuf_t *sp)
{
	int ret;
	sem_wait(sp->items);
	sem_wait(sp->mutex);
	ret = sp->buf[(++sp->front)%(sp->size)];
	sem_post(sp->mutex);
	sem_post(sp->slots);
	return ret;
}

/* sbuf_destroy(sbuf_t *)
 * delete a buf
 * return value: 0 for success, -1 for failure
 */
int sbuf_destroy(sbuf_t *sp)
{
	free(sp->buf);
	sp->size = 0;
	sp->front = sp->rear = 0;
	sem_unlink("/tmp/items");
	sem_unlink("/tmp/mutex");
	sem_unlink("/tmp/slots");
	return 0;
}
