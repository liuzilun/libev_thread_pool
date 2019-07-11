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
	//�Է�����ϵ���������������п����ղ��� EOF��Ϣ
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
		
		//���Դ���
		if (strstr(data,"quit"))
		{
			bufferevent_free(bev);
			delete cmd;
			break;
		}
	}
}
//��ʼ���������������߳���
bool XFtpServerCMD::Init()
{
	// ����scoket     bufferevent 
	//base   socket
	cout << "XFtpServerCMD::Init" << endl;
	bufferevent * bev = bufferevent_socket_new(base,sock,BEV_OPT_CLOSE_ON_FREE);
	bufferevent_setcb(bev, ReadCB, 0, EventCB, this);
	bufferevent_enable(bev, EV_READ | EV_WRITE);


	 //��ӳ�ʱ
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
