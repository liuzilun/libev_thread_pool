#include "XFtpLIST.h"
#include<string>
#include<event2/event.h>
#include<iostream>
#include<event2/bufferevent.h>
#include<io.h>


using namespace std;

void XFtpLIST::Event(struct bufferevent* bev, short what)
{
	cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR|BEV_EVENT_TIMEOUT" << endl;

	//�Է�����ϵ���������������п����ղ��� EOF��Ϣ
	if (what&(BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
	{
		bufferevent_free(bev);
		this->bev = 0;
	}
	else if (what&BEV_EVENT_CONNECTED)
	{
		cout << "BEV_EVENT_CONNECTED" << endl;
	}
}

void XFtpLIST::Read(struct bufferevent* bev)
{

}
void XFtpLIST::Write(struct bufferevent* bev)
{
	//�������
	ResCMD("226 Transfer complete\r\n");
	//�ر�����
	Close();
}

void XFtpLIST::Parse(std::string type, std::string msg)
{
	string resmsg = "";
	if (type == "PWD")
	{
		//257  "/" is current directory
		resmsg = "257 \"";
		resmsg += cmdTask->curDir;
		resmsg += "\" is current dir.\r\n";

		ResCMD(resmsg);
	}
	else if (type == "LIST")
	{
		//����ͨ���ظ���Ϣ��ʹ������ͨ������Ŀ¼
		//-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg
		//��������ͨ��
		ConnectPORT();


		ResCMD("150 Here comes the directory listing.\r\n");
		//string listdata = "-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg";
		string listdata = GetListData(cmdTask->rootDir + cmdTask->curDir);
		//����ͨ������
		Send(listdata);
	}
	else if (type == "CWD")//�л�Ŀ¼
	{
		//ȡ�������е�·��
		//CWD test\r\n
		int pos = msg.rfind(" ") + 1;
		//ȥ����β��\r\n
		string path = msg.substr(pos, msg.size() - pos-2);
		if (path[0] == '/')
		{
			cmdTask->curDir = path;
		}
		else
		{
			if (cmdTask->curDir[cmdTask->curDir.size() - 1] != '/')
			{
				cmdTask->curDir += "/";
				cmdTask->curDir += path + "/";
			}

		}
	
		ResCMD("250 Directory succes changed.\r\n");

	}
	else if (type == "CDUP")//�ص��ϲ�·��
	{
		//�ص���һ��  //  Debug/test.jpe  /Debug   /Debug/
		string path = cmdTask->curDir;
		//ͳһȥ����β��/
		if (path[path.size() - 1] == '/')
		{
			path = path.substr(0, path.size() - 1);
		}
		int pos = path.rfind("/");
		path = path.substr(0, pos);
		if (pos == 0)
		{
			path = "/";
		}
		cmdTask->curDir = path;
		ResCMD("250 CWD Directory succes changed.\r\n");
	}
}

std::string XFtpLIST::GetListData(std::string path)
{
	string data = "";
	_finddata_t file;//�洢�ļ���Ϣ
	//Ŀ¼������
	path += "/*.*";
	intptr_t dir = _findfirst(path.c_str(),&file);

	if (dir < 0)
	{
		return data;
	}
	do
	{
		//�Ƿ���Ŀ¼,ȥ��..   .
		string tmp = "";
		if (file.attrib&_A_SUBDIR)
		{
			if (strcmp(file.name, "." )== 0 || strcmp(file.name, "..") == 0)
			{
				continue;
			}
			tmp = "drwxrwxrwx 1 root group ";
		}
		else
		{
			tmp = "-rwxrwxrwx 1 root group ";
		}


		//�ļ���С
		char buf[1024] = { 0 };
		sprintf(buf, " %u ", file.size);
		tmp += buf;

		//Ŀ��,ʱ��
		strftime(buf, sizeof(buf) - 1," %b %d %H:%M ",localtime(&file.time_write));
		tmp += buf;
		tmp += file.name;
		tmp += "\r\n";
		data += tmp;

	} while (_findnext(dir, &file) == 0);
	

	return data;
}


