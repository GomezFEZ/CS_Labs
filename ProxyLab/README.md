
**ProxyLab**

参考了https://www.jianshu.com/p/8c4bfc9e3d3b.

并发编程,网络编程,缓存的混合体.

Proxy意为代理,要求写一个proxy,接收来自客户端的连接请求,转化格式后发送给服务器,再把服务器发来的信息回送给客户端.

> 一个HTTP请求的组成是这样的:一个请求行(request line),后面跟随零个或更多个请求报头(request header),再跟随一个空的文本行来终止报头列表. 一个请求行的形式是 
>
> method URI version
>
> ---\<\<CSAPP3e\>\>

本Lab只要求解析method为GET的请求.

所以,实现基本功能要做的事情无非是:

- 连接客户端,接收一个**请求行**,如GET http://www.cmu.edu:1234/hub/index.html HTTP/1.1
- 接收接下来的一系列**请求报头**,如Connection: close\r\nProxy-Connection: close\r\n
- 根据请求行和请求报头得到host与port,连接到服务器(host,port)
- 形成**新的请求行**`sprintf(req_line, "GET %s HTTP/1.0\r\n", urlp->path);`
- 新的请求行以及请求报头发送给服务器
- 接收服务器的回应并发给客户端

有很多的细节:

- 客户端发来的请求行中的URL可能是形如:http://www.cmu.edu:1234/hub/index.html,含有了host:www.cmu.edu和port:1234以及path:/hub/index.html
- 可能是形如http://www.cmu.edu/hub/index.html,(没有port)端口默认为80
- 可能是形如/hub/index.html,(没有port和host)端口默认为80,host一定在之后客户端发来的请求报头里,格式为`Host: www.cmu.edu`,这种情况需要在处理请求报头后才能得到host
