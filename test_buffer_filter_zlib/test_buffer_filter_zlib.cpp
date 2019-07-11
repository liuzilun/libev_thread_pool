#include<event2/event.h>
#include <iostream>
#include<event2/listener.h>
#include<string.h>
#ifndef _WIN32
#include<signal.h>
#endif

using namespace std;


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
    std::cout << "test_buffer_filter_zlib!\n"; 

	//创建event上下文
	event_base * base = event_base_new();
	if (base)
	{
		cout << "event_base_new success" << endl;
	}

	void Server(event_base* base);
	Server(base);
	void Client(event_base* base);
	Client(base);

	//事件分发处理
	event_base_dispatch(base);
#ifdef _WIN32
	WSACleanup();
#endif
	
	if (base)
	{
		event_base_free(base);
	}

	return 0;
}

