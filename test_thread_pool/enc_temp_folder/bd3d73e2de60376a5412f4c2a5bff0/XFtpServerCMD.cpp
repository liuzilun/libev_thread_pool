#include "XFtpServerCMD.h"
#include<event2/event.h>
#include<event2/bufferevent.h>
#include<string>

#include <iostream>

using namespace std;

void EventCB(struct bufferevent *bev, short what, void* arg)
{
	cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR" << endl;
	XFtpServerCMD* cmd = (XFtpServerCMD*)arg;
	//对方网络断掉，或机器死机，有可能收不到 EOF消息
	if (what&(BEV_EVENT_EOF | BEV_EVENT_ERROR|BEV_EVENT_TIMEOUT))
	{
		bufferevent_free(bev);
		delete cmd;
	}
}
static void ReadCB(bufferevent* bev, void*  arg)
{
	
	XFtpServerCMD* cmd = (XFtpServerCMD*)arg;
	char data[1024] = { 0 };
	for (;;)
	{
		int len = bufferevent_read(bev, data, sizeof(data) - 1);
		if (len <= 0)break;
		data[len] = '\0';
		cout << data << flush;
		
		//测试代码
		if (strstr(data,"quit"))
		{
			bufferevent_free(bev);
			delete cmd;
			break;
		}
	}
}
//初始化任务，运行在了线程中
bool XFtpServerCMD::Init()
{
	// 监听scoket     bufferevent 
	//base   socket
	cout << "XFtpServerCMD::Init" << endl;
	bufferevent * bev = bufferevent_socket_new(base,sock,BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev, ReadCB, 0, EventCB, this);
	bufferevent_enable(bev, EV_READ | EV_WRITE);


	 //添加超时
	timeval rt = {10,0};

	bufferevent_set_timeouts(bev, &rt, 0);
	return true;
}

XFtpServerCMD::XFtpServerCMD()
{
}


XFtpServerCMD::~XFtpServerCMD()
{
}
