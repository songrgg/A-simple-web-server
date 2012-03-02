#include "http.h"

int main(int argc, char *argv[])
{
	int clientfd, port;
	char buf[8192];
	rio_t rio;
	if (argc != 3) {
		fprintf(stderr, "usage: %s <hostname> <port>.", argv[0]);
		exit(-1);
	}
	port = atoi(argv[2]);
	clientfd = open_clientfd(argv[1], port);
	rio_readinitb(&rio, clientfd);
	while (fgets(buf, 8192, stdin) != NULL) {
		rio_writen(clientfd, buf, strlen(buf));
		rio_readlineb(&rio, buf, 8192);
		fputs(buf, stdout);
	}
	close(clientfd);
	exit(0);
}
