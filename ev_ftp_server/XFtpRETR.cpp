#include "XFtpRETR.h"
#include<string>
#include<event2/event.h>
#include<iostream>
#include<event2/bufferevent.h>

using namespace std;


void XFtpRETR::Write(struct bufferevent* bev)
{
	if (!fp) return;

	int len = fread(buf, 1, sizeof(buf), fp);
	if (len <= 0)
	{
		Close();
		ResCMD("226 Transfer complete\r\n");

		return;
	}
	cout << "[" << len << "]" << flush;
	Send(buf, len);
}
void XFtpRETR::Event(struct bufferevent* bev, short what)
{
	cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR|BEV_EVENT_TIMEOUT" << endl;

	//�Է�����ϵ���������������п����ղ��� EOF��Ϣ
	if (what&(BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
	{
		cout << "XFtpRETR BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT" << endl;
		bufferevent_free(bev);
		this->bev = 0;

		Close();
	}
	else if (what&BEV_EVENT_CONNECTED)
	{
		cout << "XFtpRETR BEV_EVENT_CONNECTED" << endl;
	}
}
void XFtpRETR::Parse(std::string type, std::string msg)
{
	//ȡ���ļ���
	int pos = msg.rfind(" ") + 1;
	string filename = msg.substr(pos, msg.size() - pos - 2);
	string path = cmdTask->rootDir;
	path += cmdTask->curDir;
	path += filename;

	fp = fopen(path.c_str(), "rb");
	if (fp)
	{
		//�򿪳ɹ�����������ͨ�������Ϳ�ʼ�����ļ�ָ��
		//�����ļ�
		ResCMD("150 File Ok \r\n");
		ConnectPORT();

		//����д���¼�
		bufferevent_trigger(bev, EV_WRITE, 0);
		
	}
	else
	{
		ResCMD("450 file open failed! \r\n");
	}
}


