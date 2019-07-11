#include<event2/event.h>
#include<event2/thread.h>
#include<event2/listener.h>
#include<event2/bufferevent.h>
#include<string.h>
#include <iostream>
#define SPORT 5001
#ifndef _WIN32
#include<signal.h>
#endif // !

using namespace std;
void read_cb(bufferevent* be, void* arg)
{
	cout << "read" << endl;
	char data[1024] = { 0 };
	//读取输入缓冲数据
	int len = bufferevent_read(be, data, sizeof(data) - 1);
	cout << "[" << data << "]" << endl;

	if (len <= 0)return;
	if (strstr(data, "quit") != NULL)
	{
		cout << "quit" << endl;
		//退出并关闭socket
		bufferevent_free(be);
	}

	//发送数据，写入到输出缓冲
	bufferevent_write(be, "OK", 3);

}
void write_cb(bufferevent* be, void* arg)
{
	cout << "write" << endl;
}
//错误，超时，连接断开会进入
void event_cb(bufferevent* be,short events, void* arg)
{
	cout << "event" << endl;
	//超时事件发生后，数据读取停止
	if (events & BEV_EVENT_TIMEOUT&&events&BEV_EVENT_READING)
	{
	if (events & BEV_EVENT_TIMEOUT&&events&BEV_EVENT_READING)
		cout << "BEV_EVENT_TIMEOUT&&BEV_EVENT_READING" << endl;
	    
	     
	    //bufferevent_enable(be, EV_READ);
		bufferevent_free(be);//超时一般断开连接
	}
	else if (events&BEV_EVENT_ERROR)
	{
		bufferevent_free(be);
	}
	else
	{
		cout << "OTHERS" << endl;
	}
}
void listen_cb(evconnlistener* ev, evutil_socket_t s, sockaddr* sin, int slen, void* arg)
{
	cout << "listen call back" << endl;
	event_base* base = (event_base*)arg;
	//BEV_OPT_CLOSE_ON_FREE 清理BUffevent时关闭socket
	bufferevent *bev = bufferevent_socket_new(base,s,BEV_OPT_CLOSE_ON_FREE);
	bufferevent_enable(bev,EV_READ | EV_WRITE);

	//设置水位
	//读取水位
	bufferevent_setwatermark(bev, EV_READ,
		5,  //低水位    0无限
		10);   //高水位  0无限

	bufferevent_setwatermark(bev, EV_WRITE,
		5,  //低水位    0无限
		10);   //高水位  0无限

	// 超时时间设置
	timeval t1 = { 3,0 };
	bufferevent_set_timeouts(bev, &t1, 0);

	//设置回调函数
	bufferevent_setcb(bev, read_cb, write_cb, event_cb, base);

}
int main()
{


#ifdef _WIN32
	//初始化 socket
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);
#else
	//忽略管道信号，发送数据给已关闭的socket
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return 1;
#endif

	   event_base* base = event_base_new();
	   //创建网络服务器
	   sockaddr_in sin;
	   memset(&sin, 0, sizeof(sin));

	   sin.sin_family = AF_INET;
	   sin.sin_port = htons(5001);

	   evconnlistener* ev = evconnlistener_new_bind(base, listen_cb, base, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
		   10,//listen back
		   (sockaddr*)&sin,
		   sizeof(sin));


		event_base_dispatch(base);
		
		event_base_free(base);
		evconnlistener_free(ev);


	return 0;
}

