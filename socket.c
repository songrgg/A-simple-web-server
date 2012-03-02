#include "http.h"

int open_clientfd(char *hostname, int port)
{
	int clientfd;
	struct sockaddr_in serveraddr;
	struct hostent *hp;

	if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;
	
	/*initialize the sockaddr*/
	if ((hp = gethostbyname(hostname)) == NULL)
		return -2;
		
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);
	bcopy((char *)hp->h_addr, (char *)&serveraddr.sin_addr.s_addr, hp->h_length);

	if (connect(clientfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0)
		return -1;
	return clientfd;
}

int open_listenfd(int port)
{
	int listenfd;
	int optval = 1;
	struct sockaddr_in serveraddr;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;


	/*allow local address reuse*/
	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0)
		return -1;
	
	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)port);

	if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0)
		return -1;
	
	if (listen(listenfd, LISTENQ) < 0)
		return -1;
	return listenfd;
}
