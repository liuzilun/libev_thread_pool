#pragma once
#include "XFtpTask.h"
class XFtpLIST :
	public XFtpTask
{
public:

	virtual void Parse(std::string type, std::string msg);
	virtual void Read(struct bufferevent* bev);
	virtual void Write(struct bufferevent* bev);

	virtual void Event(struct bufferevent* bev, short what);

private:
	std::string GetListData(std::string path);

	

};

