#include<event2/event.h>
#include <iostream>
#include<signal.h>
#define SPORT 5001
bool isexit = false;

using namespace std;
//sock 文件描述符，which 事件类型,arg 传递参数

static void ctrl_c(int sock, short which, void* arg)
{
	cout << "receive signal " << endl;
	event_base* base = (event_base* )arg;
	//执行完当前的处理事件函数就退出
	//event_base_loopbreak(base);
	
	//运行所有活动事件再退出，
	//事件循环没有运行时也要等运行一次再退出
	timeval t ={3,0};//至少运行3秒后再退出
	event_base_lopexit(base,&t);
	
}

static void kill(int sock, short which, void* arg)
{
	cout << "receive kill signal " << endl;
	event * ev = (event*)arg;
	//如果处于非待决
	if (!evsignal_pending(ev, NULL)) {
		event_del(ev);
		event_add(ev, NULL);

	}
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
	////添加事件到pengding
	if (event_add(csig, 0) != 0)
	{
		cerr << "event_add failed !" << endl;
		return -1;
	}

	//添加KILL信号
	//非持久事件，只进入一次
	/*event* ksig = event_new(base, SIGTERM, EV_SIGNAL, kill, base);
	if (!ksig)
	{
		cerr << "SIGINT evsignal_new failed " << endl;
		return -1;
	}
	if (event_add(ksig, 0) != 0)
	{
		cerr << "event_add failed !" << endl;
		return -1;
	}*/
	//进入主事件循环
	
	//event_base_dispatch(base);
	//EVLOOP_ONCE,等待一个事件运行，直到没有活动事件就退出
	//EVLOOG_NONBLOCK  有活动事件，就处理，没有就返回0
	//while(!isexit)
	//{
	//   event_base_loop(base,EVLOOP_NONBLOCK);
	//}
	
	//EVLOOP_NO_EXIT_ON_EMPTY 没有注册事件也不返回，用于事件后期多线程添加
	event_base_loop(base,EVLOOP_NO_EXIT_ON_EMPTY);

	event_free(csig);
	event_base_free(base);

	return 0;
}

