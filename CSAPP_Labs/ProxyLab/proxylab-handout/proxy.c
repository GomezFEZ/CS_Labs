#include <stdio.h>

#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";
static const char *con_hdr = "Connection: close\r\n";
static const char *pro_con_hdr = "Proxy-Connection: close\r\n";

typedef struct url_parsed {
    char host[MAXLINE];
    char port[MAXLINE];
    char path[MAXLINE];
} url_parsed;

void doit(int fd);
void parse_url(char *url_buf, url_parsed *urlp);
void uri2req_header(rio_t *rio, url_parsed *urlp, char *dest);

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
        printf("Proxy accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        Close(connfd);
    }

    return 0;
}

void doit(int fd) {
    char buf[MAXLINE], method[MAXLINE], url[MAXLINE], version[MAXLINE], req_hdr[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;
    url_parsed u;

    Rio_readinitb(&rio, fd);
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("Proxy get buf %s\n", buf);
    // now buf is like GET http://www.cmu.edu/hub/index.html HTTP/1.1
    // GET - method, http://www.cmu.edu/hub/index.html - url, HTTP/1.1 - version
    sscanf(buf, "%s%s%s", method, url, version);
    if (strcmp(method, "GET")) {
        fprintf(stderr, "Proxy can not handle this method: %s\n", method);
        return;
    }
    printf("Now parse url...\n");
    parse_url(buf, &u);
    // printf("Now transform url into request headers...\n");
    uri2req_header(&rio, &u, req_hdr);

    // connect to sercer
    int connfd = Open_clientfd(u.host, u.port);
    rio_t server_rio;
    printf("Successfully connected to server (%s:%s)\n", u.host, u.port);
    printf("Now sending request header...\n");
    Rio_readinitb(&server_rio, connfd);
    Rio_writen(connfd, &server_rio, strlen(req_hdr));

    int total = 0, cur = 0;
    while((cur = Rio_readlineb(&server_rio, buf, MAXLINE)) != 0) {
        total += cur;
        Rio_writen(fd, buf, cur);
    }
    printf("Proxy sent %d bytes to server\n", total);

    Close(connfd);
}

// get uri from like http://www.cmu.edu:8080/hub/index.html
void parse_url(char *url_buf, url_parsed *urlp) {
    // if (strstr(url_buf, "http://") != url_buf) {
    //     fprintf(stderr, "Invalid url %s\n", url_buf);
    //     return;
    // }
    // url_buf += strlen("http://");
    char *cp = strstr(url_buf, ':');
    *cp = '\0';
    strcpy(urlp->host, url_buf);
    url_buf = cp + 1;
    cp = strstr(url_buf, '/');
    *cp = '\0';
    strcpy(urlp->port, url_buf);
    *cp = '/';
    strcpy(urlp->path, cp);
}

// read the rest(buf) and turn uri+buf into Request headers
void uri2req_header(rio_t *rio, url_parsed *urlp, char *dest) {
    char buf[MAXLINE];
    char req_hdr[MAXLINE], host_hdr[MAXLINE], other[MAXLINE];

    sprintf(req_hdr, "GET %s HTTP/1.0\r\n", urlp->path);
    while (Rio_readlineb(rio, buf, MAXLINE) > 0)
        if (!strcmp(buf, "\r\n")) {
            strcat(other, "\r\n");
            break;
        } else if (strncasecmp(buf, "Host:", 5))
            strcpy(host_hdr, buf);
        else if (!strncasecmp(buf, "Connection:", 11) &&
                 !strncasecmp(buf, "Proxy_Connection:", 17) &&
                 !strncasecmp(buf, "User-agent:", 11)) {
            strcat(other, buf);
        }

    if (!strlen(host_hdr)) sprintf(host_hdr, "Host: %s\r\n", urlp->host);

    sprintf(dest, "%s%s%s%s%s%s", req_hdr, host_hdr, con_hdr, pro_con_hdr,
            user_agent_hdr, other);
    return;
}