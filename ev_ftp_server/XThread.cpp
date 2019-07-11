#include "XThread.h"
#include<iostream>
#include<event2/event.h>
#include<event2/util.h>
#include"Xtask.h"

#include<thread>
#ifdef _WIN32
#else
#include<unistd.h>
#endif 
using namespace std;

//事件回调函数，激活线程任务
static void NotifyCB(evutil_socket_t fd, short which, void* arg)
{
	XThread* t = (XThread*)arg;

	t->Notify(fd, which);
}
void XThread::Notify(evutil_socket_t fd, short which)
{
	//水平触发，只要没接收完全，会再次触发进来
	char buf[2] = { 0 };
#ifdef _WIN32
	int re = recv(fd, buf, 1, 0);
#else
	//linux 中是管道，不是用recv
	int re = read(fd, buf, 1);
#endif
	if (re <= 0)
	{
		return;
	}
	cout << id << " thread " << buf<<endl;
	//获取任务
	task_mutex.lock();
	if (tasks.empty())
	{
		task_mutex.unlock();
		return;
	}
	XTask* task = tasks.front();
	tasks.pop_front();
	task_mutex.unlock();
	task->Init();



}
//安装线程，初始化event_base 和管道监听事件用于激活
bool XThread::Setup()
{
	//windows用配对ｓｏｃｋｅｔ,linux  用管道
#ifdef _WIN32

	//创建一个socketpair 可以互相通信  fds[0] 读，fds[1]写
	evutil_socket_t fds[2];
	if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, fds) < 0)
	{
		cout << "evutil_socketpair failed! " << endl;
		return false;
	}
	//设置成非阻塞
	evutil_make_socket_nonblocking(fds[0]);
	evutil_make_socket_nonblocking(fds[1]);
#else
	//创建的管道，不能用send,recv读取  用read,write

	int  fds[2];
	if (pipe(fds))
	{
		cerr << "fpie failed! " << endl;
		return false;
	}
#endif 
	//读取绑定到event 事件中，写入要保存
	notify_send_fd = fds[1];

	//创建libevent 上下文（无锁）
	event_config* ev_config = event_config_new();
	event_config_set_flag(ev_config, EVENT_BASE_FLAG_NOLOCK);

	this->base = event_base_new_with_config(ev_config);
	if (!base)
	{
		cerr << "event_base_new_with_config failded in thread!" << endl;
		return false;
	}
	
	
	event_config_free(ev_config);
	//添加管道监听事件，用于激活线程执行任务
	event* ev = event_new(base, fds[0], EV_READ | EV_PERSIST, NotifyCB, this);
	event_add(ev, 0);
	return true;
}
//启动线程
void XThread::Start()
{
	//启动线程
	thread th(&XThread::Main, this);

	//清理主线程与子线程联系
	th.detach();
}
//线程激活
void XThread::Activate()
{
#ifdef _WIN32
	int re = send(this->notify_send_fd, "c", 1, 0);

#else
	int re = write(this->notify_send_fd, "c", 1);

#endif
	if (re <= 0)
	{
		cerr << "XThread::Activate() failed" << endl;
	}
}

//线程函数
void XThread::Main()
{
	cout << id << "XThread:Main  begin" << id<<endl;
	event_base_dispatch(base);

	event_base_free(base);
	cout << id << "XThread:Main  end" <<id<< endl;
}
void XThread::AddTask(XTask* t)
{
	if (!t) return;
	t->base = this->base;
	task_mutex.lock();
	tasks.push_back(t);
	task_mutex.unlock();
}

XThread::XThread()
{
}


XThread::~XThread()
{
}
