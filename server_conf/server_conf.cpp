#include<event2/event.h>
#include<event2/thread.h>
#include<event2/listener.h>
#include <iostream>
#define SPORT 5001
#ifndef _WIN32
#include<signal.h>
#endif // !

using namespace std;


void listen_cb(struct evconnlistener* e, evutil_socket_t s, struct sockaddr* a, int socklen, void* arg)
{
	cout << "call back!" << endl;
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

	//初始化配置libevent 上下文

	//创建配置上下文
	event_config* conf = event_config_new();
	//显示支持的网络模式
	const char** methods = event_get_supported_methods();

	cout << "event_get_supported_methods: " << endl;
	for (int i = 0; methods[i]!= NULL; i++)
	{
		cout << methods[i] << endl;
	}

	//设置特征
	//设置了FDS 其化特征就无法设置，在windows 中EV_FEATURE_FDS无效

	//event_config_require_features(conf,EV_FEATURE_FDS);//EV_FEATURE_FDS不支持epoll
		
													   //设置网络模型select
	//event_config_avoid_method(conf, "epoll");
	//event_config_avoid_method(conf, "poll");

	//Windows中支持IOCP (线程池)
#ifdef _WIN32
	event_config_set_flag(conf, EVENT_BASE_FLAG_STARTUP_IOCP);
	//初始化IPCO的线程
	evthread_use_windows_threads();
	//设置CPU数量
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	event_config_set_num_cpus_hint(conf, si.dwNumberOfProcessors);
#endif 




	event_base* base = event_base_new_with_config(conf);
	event_config_free(conf);



	if (!base)
	{
		cerr << "event base new with config failed!" << endl;
		base = event_base_new();
		if (!base)
		{
			cerr << "event_base_new failed" << endl;
			return 0;
		}
	}
	else
	{
		
		//获取当前网络模型
		cout << "current method is " << event_base_get_method(base) << endl;
		//确认特征是否生效
		int f = event_base_get_features(base);
		if (f&EV_FEATURE_ET)
		{
			cout << "EV_FEATURE_ET supported." << endl;
		}
		else
		{
			cout << "EV_FEATURE_ET not supported." << endl;
		}

		if (f&EV_FEATURE_O1)
		{
			cout << "EV_FEATURE_O1 supported." << endl;
		}
		else
		{
			cout << "EV_FEATURE_O1 not supported." << endl;
		}
		if (f&EV_FEATURE_FDS)
		{
			cout << "EV_FEATURE_FDS supported." << endl;
		}
		else
		{
			cout << "EV_FEATURE_FDS not supported." << endl;
		}
		if (f&EV_FEATURE_EARLY_CLOSE)
		{
			cout << "EV_FEATURE_EARLY_CLOSE supported." << endl;
		}
		else
		{
			cout << "EV_FEATURE_EARLY_CLOSE not supported." << endl;
		}


		sockaddr_in sin;
		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(SPORT);
		evconnlistener* ev = evconnlistener_new_bind(base, listen_cb, base, 10, LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, (sockaddr*)&sin, sizeof(sin));
		event_base_dispatch(base);
		evconnlistener_free(ev);




		event_base_free(base);
		cout << "event_base_new_with_config success " << endl;
	}

	return 0;
}

