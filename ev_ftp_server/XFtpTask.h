#pragma once
#include "XTask.h"
#include<string>


class XFtpTask :
	public XTask
{
public:
	//解析协议
	virtual void Parse(std::string type,std::string msg){}

	//回复ｃｍｄ消息
	void ResCMD(std::string msg);

	//发送建了连接的数据通道
	void Send(std::string data);
	void Send(const char* data, int datasize);

	void Close();

	//连接数据通道
	void ConnectPORT();

	virtual void Read(struct bufferevent* bev){}
	virtual void Write(struct bufferevent* bev) {}
	virtual void Event(struct bufferevent* bev,short what) {}

	void SetCallback(struct bufferevent* bev);
	bool Init() { return true; }



	std::string curDir = "/";
	std::string rootDir = ".";

	//PORT数据通道的IP和端口
	std::string ip = "";
	int port = 0;

	//命令通道
	XFtpTask* cmdTask = 0;
protected:
	static void ReadCB(bufferevent* bev, void*  arg);
	static void EventCB(struct bufferevent *bev, short what, void* arg);
	static void WriteCB(bufferevent* bev, void*  arg);

	//bev
	struct bufferevent* bev = 0;
	FILE * fp = 0;

};

