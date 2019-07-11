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

	//对方网络断掉，或机器死机，有可能收不到 EOF消息
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
	//发送完成
	ResCMD("226 Transfer complete\r\n");
	//关闭连接
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
		//命令通道回复消息，使用数据通道发送目录
		//-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg
		//连接数据通道
		ConnectPORT();


		ResCMD("150 Here comes the directory listing.\r\n");
		//string listdata = "-rwxrwxrwx 1 root group 64463 Mar 14 09:53 101.jpg";
		string listdata = GetListData(cmdTask->rootDir + cmdTask->curDir);
		//数据通道发送
		Send(listdata);
	}
	else if (type == "CWD")//切换目录
	{
		//取出命令中的路径
		//CWD test\r\n
		int pos = msg.rfind(" ") + 1;
		//去掉结尾的\r\n
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
	else if (type == "CDUP")//回到上层路径
	{
		//回到上一层  //  Debug/test.jpe  /Debug   /Debug/
		string path = cmdTask->curDir;
		//统一去掉结尾的/
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
	_finddata_t file;//存储文件信息
	//目录上下文
	path += "/*.*";
	intptr_t dir = _findfirst(path.c_str(),&file);

	if (dir < 0)
	{
		return data;
	}
	do
	{
		//是否是目录,去掉..   .
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


		//文件大小
		char buf[1024] = { 0 };
		sprintf(buf, " %u ", file.size);
		tmp += buf;

		//目期,时间
		strftime(buf, sizeof(buf) - 1," %b %d %H:%M ",localtime(&file.time_write));
		tmp += buf;
		tmp += file.name;
		tmp += "\r\n";
		data += tmp;

	} while (_findnext(dir, &file) == 0);
	

	return data;
}


