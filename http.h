#ifndef __HTTP_H__
#define __HTTP_H__

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <strings.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>

#define MAXLINE 8192

/*socket.c*/
#define LISTENQ 1024
typedef struct sockaddr SA;
int open_listenfd(int port);
int open_clientfd(char *hostname, int port);

/*rio.c*/
#define RIO_BUFSIZE 8192
typedef struct {
	int rio_fd;
	int rio_cnt;
	char *rio_bufptr;
	char rio_buf[RIO_BUFSIZE];
} rio_t;

ssize_t rio_readn(int fd, void *usrbuf, size_t n);
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
void rio_readinitb(rio_t *rp, int fd);
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen);
ssize_t rio_readnb(rio_t *rp, void *usrbuf, ssize_t n);

/*sbuf.c*/
typedef struct {
	int *buf;		/*buffer*/
	int size;		/*buffer size*/
	int front;
	int rear;
	sem_t *mutex;
	sem_t *slots; /*available slots (Insert)*/
	sem_t *items; /*available items (Remove)*/
} sbuf_t;

void sbuf_init(sbuf_t *sp, int size);  /*sem_buf initialize*/
void sbuf_insert(sbuf_t *sp, int value); 	/*add items to the buffer*/
int sbuf_remove(sbuf_t *sp);  /*get items from the buffer*/
int sbuf_destroy(sbuf_t *sp);

#endif
