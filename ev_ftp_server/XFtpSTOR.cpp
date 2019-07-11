#include "XFtpSTOR.h"
#include<string>
#include<iostream>
#include<event2/bufferevent.h>
#include<event2/event.h>

using namespace std;

void XFtpSTOR::Read(struct bufferevent* bev)
{
	if (!fp)
	{
		cout << "文件保存失败 " << endl;
		return;
	}
	for (;;)
	{
		int len = bufferevent_read(bev, buf, sizeof(buf));
		if (len <= 0)
		{
			return;
		}
		int size = fwrite(buf, 1, len, fp);
		//cout << "<" << len << ":" << size << ">" << flush;

	}
}

//解析协议
void XFtpSTOR::Parse(std::string type, std::string msg)
{
	//取出文件名
	int pos = msg.rfind(" ") + 1;
	string filename = msg.substr(pos, msg.size() - pos - 2);
	string path = cmdTask->rootDir;
	path += cmdTask->curDir;
	path += filename;

	fp = fopen(path.c_str(), "wb");
	if (fp)
	{
		//打开成功，连接数据通道，发送开始接收文件指令
		ResCMD("125 File Ok \r\n");
		ConnectPORT();

		//触发读取事件
		//bufferevent_trigger(bev, EV_READ, 0);
	}
	else
	{
		ResCMD("450 file open failed! \r\n");
	}
}
void XFtpSTOR::Event(struct bufferevent* bev, short what)
{
	cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR|BEV_EVENT_TIMEOUT" << endl;

	//对方网络断掉，或机器死机，有可能收不到 EOF消息
	if (what&(BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
	{
		cout << "XFtpSTOR BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT" << endl;
		Close();

		ResCMD("226 Transfer complete\r\n");
	}
	else if (what&BEV_EVENT_CONNECTED)
	{
		cout << "XFtpSTOR::Event BEV_EVENT_CONNECTED" << endl;
	}
}

XFtpSTOR::XFtpSTOR()
{
}


XFtpSTOR::~XFtpSTOR()
{
}
