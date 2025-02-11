#include "XFtpServerCMD.h"
#include<event2/event.h>
#include<event2/bufferevent.h>
#include<string.h>
#include<string>
#include <iostream>

using namespace std;

void XFtpServerCMD::Reg(std::string cmd, XFtpTask* call)
{
	//
	if (!call)
	{
		cout << "XFtpServerCMD::Reg call is null " << endl;
		return;
	}
	if (cmd.empty())
	{
		cout << "XFtpServerCMD::Reg cmd is null " << endl;
		return;
	}
	//已注册的的不覆盖，提示错误
	if (calls.find(cmd) != calls.end())
	{
		cout << cmd << " is already register  " << endl;
		return;
	}
	calls[cmd] = call;

	//用来做空间清理
	calls_del[call] = 0;

}
void XFtpServerCMD::Read(struct bufferevent* bev)
{
	char data[1024] = { 0 };
	for (;;)
	{
		int len = bufferevent_read(bev, data, sizeof(data) - 1);
		if (len <= 0)break;
		data[len] = '\0';
		cout <<"Recv CMD: "<< data << flush;
		//分发到处理对象
		//分析出类型 USER anonymous
		string type = "";

		for (int i = 0; i < len; i++)
		{
			if (data[i] == ' ' || data[i] == '\r')
				break;
			type += data[i];
		}
		cout << "type is [" <<type<<"]" <<endl;
		if (calls.find(type) != calls.end())
		{
			XFtpTask* t = calls[type];

			t->cmdTask = this;  //用来处理回复命令和目录
			t->ip = ip;
			t->port = port;
			t->base = this->base;
			t->Parse(type, data);

			if (type == "PORT")
			{
				ip = t->ip;
				port = t->port;
			}
		}
		else
		{
			string msg = "200 OK\r\n";
			bufferevent_write(bev, msg.c_str(), msg.size());
		}
	}
}
void XFtpServerCMD::Event(struct bufferevent* bev, short what)
{
	cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR|BEV_EVENT_TIMEOUT" << endl;

	//对方网络断掉，或机器死机，有可能收不到 EOF消息
	if (what&(BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
	{
		delete this;
	}
}
//初始化任务，运行在了线程中
bool XFtpServerCMD::Init()
{
	// 监听scoket     bufferevent 
	//base   socket
	cout << "XFtpServerCMD::Init" << endl;
	bufferevent * bev = bufferevent_socket_new(base,sock,BEV_OPT_CLOSE_ON_FREE);
	
	if (!bev)
	{
		delete this;
		return false;
	}
	this->bev = bev;
	this->SetCallback(bev);


	 //添加超时
	timeval rt = {60,0};
	bufferevent_set_timeouts(bev, &rt, 0);

	string msg = "220 Welcome to libevent XFtpServer\r\n";

	cout << "send welcom msg :" << msg << endl;
	bufferevent_write(bev, msg.c_str(), msg.size());
	return true;
}

XFtpServerCMD::XFtpServerCMD()
{
}


XFtpServerCMD::~XFtpServerCMD()
{
	Close();
	for (auto ptr = calls_del.begin(); ptr != calls_del.end(); ptr++)
	{
		ptr->first->Close();
		delete ptr->first;
	}
}
