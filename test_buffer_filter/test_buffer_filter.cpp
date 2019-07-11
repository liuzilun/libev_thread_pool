#include<event2/event.h>
#include <iostream>
#include<event2/listener.h>
#include<event2/bufferevent.h>
#include<event2/buffer.h>
#include<string.h>
#ifndef _WIN32
#include<signal.h>
#endif

using namespace std;
#define SPORT 5001

bufferevent_filter_result filter_in(evbuffer* s, evbuffer *d, ev_ssize_t limit,
	bufferevent_flush_mode mode, void* arg)
{
	cout << "filter_in" << endl;
	//读取并清理原数据
	char data[1024] = { 0 };
	int len = evbuffer_remove(s, data, sizeof(data) - 1);

	//过滤功能，所有字母转成大写

	for (int i = 0; i < len; ++i)
	{
		data[i] = toupper(data[i]);
	}
	evbuffer_add(d, data, len);
	return BEV_OK;

}
bufferevent_filter_result filter_out(evbuffer* s, evbuffer *d, ev_ssize_t limit,
	bufferevent_flush_mode mode, void* arg)
{
	cout << "filter_out" << endl;
	char data[1024] = { 0 };
	int len = evbuffer_remove(s, data, sizeof(data) - 1);

	//过滤功能，加头信息

	string str = "";
	str += "==================\n";
	str += data;
	str += "==================\n";

	evbuffer_add(d, str.c_str(), str.size());

	return BEV_OK;

}

void read_cb(bufferevent* bev, void* arg)
{
	cout << "read_cb" << endl;
	char data[1024] = { 0 };
	int len = bufferevent_read(bev, data, sizeof(data) - 1);
	cout << data << endl;
	//回复客户消息，经过输出过滤
	bufferevent_write(bev, data, len);
}
void write_cb(bufferevent* bev, void* arg)
{
	cout << "write_cb" << endl;
}
void event_cb(bufferevent* bev, short events,void* arg)
{
	cout << "event_cb" << endl;
}
void listen_cb(struct evconnlistener* e, evutil_socket_t s, struct sockaddr* a, int socklen, void* arg)
{
	cout << "call back!" << endl;

	//创建bufferevent 绑定 bufferevent filter
	bufferevent* bev = bufferevent_socket_new((event_base*)arg, s, BEV_OPT_CLOSE_ON_FREE);
	//绑定bufferevent filter
	bufferevent* bev_filter = bufferevent_filter_new(bev, filter_in,
		filter_out,
		BEV_OPT_CLOSE_ON_FREE,//关闭filter同时关闭 bufferevent
		0,
		0);
	//设置bufferevent 的回调
	bufferevent_setcb(bev_filter, read_cb, write_cb, event_cb,
		NULL);//回调函数的参数
	//开启读写
	bufferevent_enable(bev_filter, EV_READ | EV_WRITE);


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
    std::cout << "Test buffer_filter!\n"; 

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

