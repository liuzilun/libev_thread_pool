#pragma once
#include<event2/event.h>
#include<list>
#include<mutex>


class XTask;
class XThread
{
public:

	//启动线程
	void Start();
	
	//线程函数
	void Main();

	//安装线程，初始化event_base 和管道监听事件用于激活
	bool Setup();

	XThread();
	~XThread();

	//收到主线程发出的激活消息（线程池的分发）
	void Notify(evutil_socket_t fd, short which);

	//线程激活
	void Activate();
	//添加处理的任务，一个线程同时可以处理多个任务，共用一个event_base
	void AddTask(XTask* t);

	//线程编号
	int id = 0;
private:
	int notify_send_fd = 0;
	struct event_base* base = 0;
	
	//任务列表,需要考虑线程安全
	std::list<XTask*> tasks;

	std::mutex task_mutex;


};

