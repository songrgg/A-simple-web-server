#include "http.h"

#define DEBUG

void doit(int connfd);
void clienterror(int fd, int errcode, char *shortmsg, char *longmsg);
void read_requesthdrs(rio_t *rio);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void get_filetype(char *filename, char *filetype);

int main(int argc, char **argv)
{
	int listenfd, connfd, port;
	struct sockaddr clientaddr;
	int clientlen;
	char buf[8192];
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>.\n", argv[0]);
		exit(-1);
	}
	port = atoi(argv[1]);
	listenfd = open_listenfd(port);

	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);

		/*handle the connfd*/
		doit(connfd);

		close(connfd);
	}
	exit(0);
}

void doit(int fd)
{
	char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
	char filename[MAXLINE], cgiargs[MAXLINE];
	rio_t rio;
	int is_static, filesize;
	struct stat sbuf;
	/*analyse the request*/
	rio_readinitb(&rio, fd);
	rio_readlineb(&rio, buf, MAXLINE);

#ifdef DEBUG
	printf("%s", buf);
#endif
	sscanf(buf, "%s%s%s", method, uri, version);
	if (strcasecmp(method, "GET")) {
		clienterror(fd, 501, "Not Implemented", "SS does Implemented this method");
		return;
	}

	/*eliminate the request headers*/
	read_requesthdrs(&rio);

	/*analyse the file requested*/
	is_static = parse_uri(uri, filename, cgiargs);

	/*service support*/
	/*divided to static and dynamic*/
	if (stat(filename, &sbuf) < 0) {
		strcat(filename, "doesn't exist!");
		clienterror(fd, 404, "Not Found", filename);
		return;
	}
	if (is_static) {
		fflush(stdout);
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
			strcat(filename, "couldn't be accessed!");
			clienterror(fd, 403, "Forbidden", filename);
			return;
		}
		serve_static(fd, filename, sbuf.st_size);
	}
	else {
		printf("request is dynamic\n");
		fflush(stdout);
		if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
			strcat(filename, "couldn't be executed!");
			clienterror(fd, 403, "Forbidden", filename);
			return;
		}
		serve_dynamic(fd, filename, cgiargs);
	}
}

void clienterror(int fd, int errcode, char *shortmsg, char *longmsg)
{
	char header[MAXLINE], body[MAXLINE];
	sprintf(header, "HTTP/1.1 %d Not Found\r\n", errcode);
	rio_writen(fd, header, strlen(header));
	sprintf(header, "Content-type: text/html\r\n");
	rio_writen(fd, header, strlen(header));
	sprintf(body, "<html><title>SS Error</title>");
	sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
	sprintf(body, "%s %d: %s\r\n", body, errcode, shortmsg);
	sprintf(body, "%s<p>%s\r\n", body, longmsg);
	sprintf(header, "Content-length: %d\r\n\r\n", (int)strlen(body));
	rio_writen(fd, header, strlen(header));
	rio_writen(fd, body, strlen(body));
}

void read_requesthdrs(rio_t *rp)
{
	char buf[MAXLINE];

	rio_readlineb(rp, buf, MAXLINE);
	while (strcmp(buf, "\r\n"))
		rio_readlineb(rp, buf, MAXLINE);
	return;
}

/*process uri and seek filename, cgiargs*/
int parse_uri(char *uri, char *filename, char *cgiargs)
{
	char *ptr;
	if (!strstr(uri, "cgi-bin")) {  /*static content*/
		strcpy(cgiargs, "");
		strcpy(filename, "."); 		/*current directory*/
		strcat(filename, uri);
		if (uri[strlen(uri)-1] == '/')
			strcat(filename, uri);
		return 1;
	}
	else {
		ptr = index(uri, '?');
		if (ptr) { /*get the parameter*/
			strcpy(cgiargs, ptr+1);
			*ptr = '\0';
		}
		else
			strcpy(cgiargs, "");
		strcpy(filename, ".");
		strcat(filename, uri);
		return 0;
	}
}

/*send static content in the body*/
void serve_static(int fd, char *filename, int filesize)
{
	char filetype[MAXLINE];
	char buf[MAXLINE];
	char *srcp;
	int srcfd;

	get_filetype(filename, filetype);
	sprintf(buf, "HTTP/1.1 200 OK\r\n");
	sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
	sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
	sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
	rio_writen(fd, buf, strlen(buf));

	/*send response body to client*/
	srcfd = open(filename, O_RDONLY, 0);
	srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
	close(srcfd);
	rio_writen(fd, srcp, filesize);
	munmap(srcp, filesize);
}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
	pid_t pid;
	char buf[MAXLINE], *emptylist = { NULL };
	extern char **environ;

	if ((pid = fork()) < 0) {
		fprintf(stderr, "fork error!\n");
		return;
	}
	else if (pid == 0) {
		setenv("QUERY_STRING", cgiargs, 1);
		sprintf(buf, "HTTP/1.1 200 OK\r\n");
		rio_writen(fd, buf, strlen(buf));
		sprintf(buf, "Server: SS Web\r\n");
		rio_writen(fd, buf, strlen(buf));
		dup2(fd, STDOUT_FILENO);
		execve(filename, (char **)emptylist, environ);
	}
	else {
		waitpid(pid, NULL, 0);
	}
}

void get_filetype(char *filename, char *filetype)
{
	if (strstr(filename, ".html"))
		strcpy(filetype, "text/html");
	else if (strstr(filename, ".gif"))
		strcpy(filetype, "image/gif");
	else if (strstr(filename, ".jpg"))
		strcpy(filetype, "image/jpeg");
	else
		strcpy(filetype, "text/plain");
}
