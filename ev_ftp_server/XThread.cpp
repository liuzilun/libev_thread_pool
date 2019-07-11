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

//�¼��ص������������߳�����
static void NotifyCB(evutil_socket_t fd, short which, void* arg)
{
	XThread* t = (XThread*)arg;

	t->Notify(fd, which);
}
void XThread::Notify(evutil_socket_t fd, short which)
{
	//ˮƽ������ֻҪû������ȫ�����ٴδ�������
	char buf[2] = { 0 };
#ifdef _WIN32
	int re = recv(fd, buf, 1, 0);
#else
	//linux ���ǹܵ���������recv
	int re = read(fd, buf, 1);
#endif
	if (re <= 0)
	{
		return;
	}
	cout << id << " thread " << buf<<endl;
	//��ȡ����
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
//��װ�̣߳���ʼ��event_base �͹ܵ������¼����ڼ���
bool XThread::Setup()
{
	//windows����ԣ������,linux  �ùܵ�
#ifdef _WIN32

	//����һ��socketpair ���Ի���ͨ��  fds[0] ����fds[1]д
	evutil_socket_t fds[2];
	if (evutil_socketpair(AF_INET, SOCK_STREAM, 0, fds) < 0)
	{
		cout << "evutil_socketpair failed! " << endl;
		return false;
	}
	//���óɷ�����
	evutil_make_socket_nonblocking(fds[0]);
	evutil_make_socket_nonblocking(fds[1]);
#else
	//�����Ĺܵ���������send,recv��ȡ  ��read,write

	int  fds[2];
	if (pipe(fds))
	{
		cerr << "fpie failed! " << endl;
		return false;
	}
#endif 
	//��ȡ�󶨵�event �¼��У�д��Ҫ����
	notify_send_fd = fds[1];

	//����libevent �����ģ�������
	event_config* ev_config = event_config_new();
	event_config_set_flag(ev_config, EVENT_BASE_FLAG_NOLOCK);

	this->base = event_base_new_with_config(ev_config);
	if (!base)
	{
		cerr << "event_base_new_with_config failded in thread!" << endl;
		return false;
	}
	
	
	event_config_free(ev_config);
	//��ӹܵ������¼������ڼ����߳�ִ������
	event* ev = event_new(base, fds[0], EV_READ | EV_PERSIST, NotifyCB, this);
	event_add(ev, 0);
	return true;
}
//�����߳�
void XThread::Start()
{
	//�����߳�
	thread th(&XThread::Main, this);

	//�������߳������߳���ϵ
	th.detach();
}
//�̼߳���
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

//�̺߳���
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
