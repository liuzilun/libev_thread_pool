#pragma once
#include "XFtpTask.h"
class XFtpSTOR :
	public XFtpTask
{
public:

	virtual void Read(struct bufferevent* bev);

	//Ω‚Œˆ–≠“È
	virtual void Parse(std::string type, std::string msg);
	virtual void Event(struct bufferevent* bev, short what);
	XFtpSTOR();
	~XFtpSTOR();


private:
	char buf[1024] = { 0 };
};

