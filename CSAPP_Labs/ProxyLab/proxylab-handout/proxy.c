#include <stdio.h>

#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

typedef struct url_parsed {
    char host[MAXLINE];
    char path[MAXLINE];
    char User_Agent[MAXLINE];
    char Connection[MAXLINE];
    char Proxy_Connection[MAXLINE];
} url_parsed;

void doit(int fd);

int main(int argc, int *argv[]) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    printf("%s", user_agent_hdr);

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port,
                    MAXLINE, 0);
        printf("Proxy accept connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        Close(connfd);
    }

    return 0;
}

void doit(int fd) {
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;
    url_parsed u;

    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    // now buf is like GET http://www.cmu.edu/hub/index.html HTTP/1.1
    // GET - method, http://www.cmu.edu/hub/index.html - url, HTTP/1.1 - version
    sscanf(buf, "%s%s%s", method, url, version);
    

}
