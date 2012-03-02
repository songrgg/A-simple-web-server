#include "http.h"

void *thread(void *arg);
void echo(int connfd);
sbuf_t sbuf;

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>.\n", argv[0]);
		exit(1);
	}

	int port = atoi(argv[1]);
	int listenfd = open_listenfd(port);
	struct sockaddr_in clientaddr;
	int clientlen, connfd;

	int i;
	pthread_t tid;

	sbuf_destroy(&sbuf);
	sbuf_init(&sbuf, 1);

	sbuf_insert(&sbuf, 1);
	for (i=0; i<1; i++)
		sbuf_remove(&sbuf);

	for (i=0; i<4; i++)
		pthread_create(&tid, NULL, thread, &sbuf);

	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = accept(listenfd, (SA *)&clientaddr, (int *)&clientlen);
		sbuf_insert(&sbuf, connfd);
	}
}

void *thread(void *arg)
{
	/* make this thread detached, its resource will be recycled when it ends!!!
	 * because the server needn't to wait to their threads.*/
	pthread_detach(pthread_self());
	while (1) {
		int connfd = sbuf_remove(&sbuf);
		echo(connfd);
		close(connfd);
	}
	return NULL;
}


void echo(int connfd)
{
	size_t n;
	char buf[8192];
	rio_t rio;

	rio_readinitb(&rio, connfd);
	while ((n = rio_readlineb(&rio, buf, 8192)) != 0) {
		printf("server received %d bytes\n", (int)n);
		rio_writen(connfd, buf, n);
	}
}

