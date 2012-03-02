#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXLINE 8192

int main(int argc, char **argv)
{
	char *buf, *p;
	char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
	int n1, n2;

	buf = getenv("QUERY_STRING");
	sscanf(buf, "num1=%d&num2=%d", &n1, &n2);

	sprintf(content, "<html><title>ADDDDER</title>");
	sprintf(content, "%s<body>The sum of %d and %d is %d\r\n", content, n1, n2, n1+n2);
	sprintf(content, "%s</body><body>Thanks for visiting!!!</body></html>\r\n", content);

	printf("Content-length: %d\r\n", (int)strlen(content));
	printf("Content-type: text/html\r\n\r\n");
	printf("%s", content);
	fflush(stdout);
	exit(0);
}
