#include<event2/event.h>
#include <iostream>
#include<event2/listener.h>
#include<string.h>
#include<thread>
#include<errno.h>
#ifndef _WIN32
#include<signal.h>
#endif
#define SPORT 5001

using namespace std;

//正常断开，超时会进入,从事件里面判断处理
void client_cb(evutil_socket_t s, short w, void *arg)
{
	//水平触发LT 只要有数据没有处理，会一直进入
	//边缘触发有数据时进入一次
	cout << "client cb" << flush; 
	//超时事件
	// 正常断开，超时会进入, 从事件里面判断处理，注意清理时机
	//判断完全所有事件
	if (w&EV_TIMEOUT)
	{
		cout << "time out " << endl;
		event_free((event*)arg);
		evutil_closesocket(s);
		return;
	}
	//需要清理event

	char buf[5] = { 0 };
	//char buf[1024] = { 0 };
	int len = recv(s, buf, sizeof(buf) - 1, 0);
	if (len > 0)
	{
		cout << buf << endl;
		send(s, "ok", 2, 0);
	}
	else
	{
		cout << "." << flush;
		event_free((event*)arg);
		evutil_closesocket(s);
	}

}


void listen_cb(evutil_socket_t s, short w, void *arg) 
{
	cout << "listen" << endl;
	sockaddr_in sin_r;
	socklen_t size = sizeof(sin_r);
	//读取连接信息
	evutil_socket_t client = accept(s, (sockaddr*)&sin_r ,&size);
	char ip[16] = { 0 };
	evutil_inet_ntop(AF_INET, &sin_r.sin_addr, ip, sizeof(ip));
	cout << "receive client ip is :" << ip << endl;

	//客户端读取事件
	//1.水平触发，只要有数据没有处理，会一直进入
	//2.边缘触发只发送一次，只进入一次
	//event* ev = event_new((event_base*)arg, client, EV_READ | EV_PERSIST, client_cb, event_self_cbarg());
	event* ev = event_new((event_base*)arg, client, EV_READ | EV_PERSIST|EV_ET, client_cb, event_self_cbarg());

	timeval t = { 10,0 };
	event_add(ev, &t);

}
int main(int argc,char* argv[])
{
#ifdef _WIN32
	//初始化 socket
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
#else
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		return 1;
	}
#endif

	event_base* base = event_base_new();

	//定时器
	cout << "test event server" << endl;

	evutil_socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock <= 0)
	{
		cout << "socket error!" << strerror(errno)<<endl;
	}
	//设置地址复用和非阻塞
	evutil_make_socket_nonblocking(sock);
	evutil_make_listen_socket_reuseable(sock);


	sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));

	sin.sin_family = AF_INET;
	sin.sin_port = htons(SPORT);

	//绑定端口和地址，使用全局的bind std中也有一个bind
	int re = ::bind(sock, (sockaddr*)&sin, sizeof(sin));
	if (re != 0)
	{
		cerr << "bind error" << strerror(errno) << endl;
		return -1;
	}
	//开始监听
	listen(sock, 10);

	//开始接收连接事件，默认水平触发
	event* ev = event_new(base, sock, EV_READ | EV_PERSIST, listen_cb, base);
	event_add(ev, 0);

	//事件分发处理
	event_base_dispatch(base);
	event_base_free(base);
	evutil_closesocket(sock);
	return 0;
}

