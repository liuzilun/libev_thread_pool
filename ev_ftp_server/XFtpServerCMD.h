#pragma once
#include"XTask.h"
#include"XFtpTask.h"
#include<map>
class XFtpServerCMD:public XFtpTask
{
public:
	//初始化
	virtual bool Init();
	virtual void Read(struct bufferevent* bev);
	virtual void Event(struct bufferevent* bev, short what);

	//注册命令处理对象,不需要考虑线程安全，注册先，调用时还未分发到线程
	void Reg(std::string, XFtpTask* call);

	XFtpServerCMD();
	~XFtpServerCMD();

private:
	//注册的对象
	std::map<std::string, XFtpTask*>calls;

	//用来做空间清理
	std::map<XFtpTask*, int>calls_del;

};

