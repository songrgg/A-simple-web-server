#include "http.h"

ssize_t rio_readn(int fd, void *usrbuf, size_t n)
{
	char *bufptr = usrbuf;
	ssize_t nread;
	size_t nleft = n;

	while (nleft > 0) {
		if ((nread = read(fd, bufptr, nleft)) < 0) {
			if (errno == EINTR)
				nread = 0;
			else
				return -1;
		}
		else if (nread == 0)
			break;
		usrbuf += nread;
		nleft -= nread;
	}
	return (n - nleft);
}

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
	size_t nleft = n;
	char *bufptr = usrbuf;
	ssize_t nwritten;

	while (nleft > 0) {
		if ((nwritten = write(fd, bufptr, nleft)) <= 0) {
			if (errno == EINTR)
				nwritten = 0;
			else
				return -1;
		}
		nleft -= nwritten;
		bufptr += nwritten;
	}
	return n;
}

void rio_readinitb(rio_t *rp, int fd)
{
	rp->rio_fd = fd;
	rp->rio_cnt = 0;
	rp->rio_bufptr = rp->rio_buf;
}

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
	int cnt;
	/*check if the buffer is empty, if so, refill it! */
	while (rp->rio_cnt <= 0) {
		rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));

		if (rp->rio_cnt < 0) {
			if (errno != EINTR)
				return -1;
		}
		else if (rp->rio_cnt == 0)  /*EOF*/
			return 0;
		else
			rp->rio_bufptr = rp->rio_buf;  /*reset*/
	}

	/*memcpy the data in the buf to the usrbuf*/
	cnt = n;
	if (cnt > rp->rio_cnt)
		cnt = rp->rio_cnt;
	memcpy(usrbuf, rp->rio_bufptr, cnt);
	rp->rio_bufptr += cnt;
	rp->rio_cnt -= cnt;
	return cnt;
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen)
{
	int n, rc;
	char c, *bufptr = usrbuf;
	for (n = 1; n < maxlen; n++) {
		if ((rc = rio_read(rp, &c, 1)) == 1) {
			*bufptr++ = c;
			if (c == '\n')
				break;
		}
		/*EOF*/
		else if (rc == 0) {
			if (n == 1)
				return 0;  /*no data read*/
			else
				break; /*some data read*/
		}
		else
			return -1;  /*error*/
	}
	*bufptr = 0;
	return n;
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, ssize_t n)
{
	ssize_t nread;
	ssize_t nleft = n;
	char *bufptr = usrbuf;

	while (nleft > 0) {
		if ((nread = rio_read(rp, bufptr, n)) < 0) {
			if (errno == EINTR)
				nread = 0;
			else
				return -1;
		}
		else if (nread == 0)
			break;
		bufptr += nread;
		nleft -= nread;
	}
	return (n - nleft);
}
