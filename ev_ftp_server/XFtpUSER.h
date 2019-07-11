#pragma once
#include "XFtpTask.h"
class XFtpUSER :
	public XFtpTask
{

	virtual void Parse(std::string type, std::string msg);
public:
	XFtpUSER();
	~XFtpUSER();
};

