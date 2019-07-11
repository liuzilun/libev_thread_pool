#include<event2/event.h>
#include <iostream>
#include<event2/listener.h>
#include<string.h>
#include<thread>
#ifndef _WIN32
#include<signal.h>
#endif

using namespace std;
static timeval t1 = {1,0};
void timerl(int sock, short which, void* arg)
{
	event* ev = (event*)arg;
	cout << "[timer call back!]" <<flush;
	if (!evtimer_pending(ev, &t1))
	{
		evtimer_del(ev);
		evtimer_add(ev, &t1);
	}
}

void timer2(int sock, short which, void* arg)
{
	event* ev = (event*)arg;
	cout << "[timer2!]" << flush;
	this_thread::sleep_for(3000ms);

}

void timer3(int sock, short which, void* arg)
{
	event* ev = (event*)arg;
	cout << "[timer333333!]" << flush;


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
	cout << "test timeer" << endl;

	/*
#define evtimer_assign(ev, b, cb, arg) \
	event_assign((ev), (b), -1, 0, (cb), (arg))
#define evtimer_new(b, cb, arg)		event_new((b), -1, 0, (cb), (arg))
#define evtimer_add(ev, tv)		event_add((ev), (tv))
#define evtimer_del(ev)			event_del(ev)
#define evtimer_pending(ev, tv)		event_pending((ev), EV_TIMEOUT, (tv))
#define evtimer_initialized(ev)		event_initialized(ev)
*/
	///定时器默认非持久事件，只进入一次
	event * evl = evtimer_new(base, timerl, event_self_cbarg());
	if (!evl)
	{
		cout << "timerl failed" << endl;
		return -1;
	}
	evtimer_add(evl, &t1);


	static timeval t2;
	t2.tv_sec = 2;
	t2.tv_usec = 0;

	event* ev2 = event_new(base, -1, EV_PERSIST, timer2, 0);
	event_add(ev2, &t2);


	//超时优化性能优化，默认event 用二叉堆存储（完全二叉树）
	//优化到双向队列，插入删除O(1)
	event* ev3= event_new(base, -1, EV_PERSIST, timer3, 0);

	static timeval tv_in = { 3,0 };
	const timeval *t3;

	//优化
	t3 = event_base_init_common_timeout(base, &tv_in);
	event_add(ev3, t3);//插入性能O(1)


	//事件分发处理
	event_base_dispatch(base);
	event_base_free(base);
	return 0;
}

