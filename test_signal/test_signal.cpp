#include<event2/event.h>

#include <iostream>

#ifndef _WIN32
#include<signal.h>
#endif


using namespace std;
//sock 文件描述符，which 事件类型,arg 传递参数

static void ctrl_c(int sock, short which, void* arg)
{
	cout << "receive signal " << endl;
}

static void kill(int sock, short which, void* arg)
{
	cout << "receive kill signal " << endl;
	event * ev = (event*)arg;
	//如果处于非待决，可以再次添加，变为待决状态
	if (!evsignal_pending(ev, NULL)) {
		event_del(ev);
		event_add(ev, NULL);

	}
	//业务上可以在特殊情况下的处理，符合某条件时，再让事件待决

}
int main(int argc,char* argv[])
{
	event_base* base = event_base_new();

	//添加ctrl+c 信号事件，处于no pending  
	//evsignal_new　隐藏的状态 EV_SIGNAL|EV_PERSIST
	event* csig = evsignal_new(base, SIGINT, ctrl_c, base);

	if (!csig)
	{
		cerr << "SIGINT evsignal_new failed " << endl;
		return -1;
	}
	//添加事件到pengding
	if (event_add(csig, 0) != 0)
	{
		cerr << "event_add failed !" << endl;
		return -1;
	}

	//添加KILL信号
	//非持久事件，只进入一次
	event* ksig = event_new(base, SIGTERM, EV_SIGNAL, kill, base);
	if (!ksig)
	{
		cerr << "SIGINT evsignal_new failed " << endl;
		return -1;
	}

	if (event_add(ksig, 0) != 0)
	{
		cerr << "event_add failed !" << endl;
		return -1;
	}
	//进入主事件循环
	event_base_dispatch(base);

	event_free(ksig);
	event_base_free(base);

	return 0;
}

