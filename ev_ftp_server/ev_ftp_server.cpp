#include<event2/event.h>
#include <iostream>
#include<event2/listener.h>
#include<string.h>
#include "XThreadPool.h"
#include"XTask.h"
#include"XFtpFactory.h"


#ifndef _WIN32
#include<signal.h>
#endif

using namespace std;
#define SPORT 21

void listen_cb(struct evconnlistener* e,evutil_socket_t s, struct sockaddr* a, int socklen, void* arg)
{
	cout << "call back!" << endl;
	XTask* t = XFtpFactory::Get()->CreateTask();

	t->sock = s;
	XThreadPool::Get()->Dispatch(t);
}

int main()
{

#ifdef _WIN32
	//初始化 socket
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);
#else
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		return 1;
	}
#endif
 //1.初始化线程池
	XThreadPool::Get()->Init(10);
	
	
	
	
	std::cout << "Test Thread!\n"; 

	//创建event上下文
	event_base * base = event_base_new();
	if (base)
	{
		cout << "event_base_new success" << endl;
	}

	//监听端口socket,bind,listen，绑定事件
	sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SPORT);

	evconnlistener *ev = evconnlistener_new_bind(base,//上下文
		listen_cb,
		base,
		LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,//地址重用，listen 关闭同时关socket
		10,//连接队列大小，对应listen函数
		(sockaddr*)&sin,  //绑定的地址和端口
		sizeof(sin)
		);

	//事件分发处理
	event_base_dispatch(base);



#ifdef _WIN32
	WSACleanup();
#endif

	if (ev)
	{
		evconnlistener_free(ev);
	}
	if (base)
	{
		event_base_free(base);
	}

	return 0;
}

