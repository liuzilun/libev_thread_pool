#pragma once
#include "XTask.h"
#include<string>


class XFtpTask :
	public XTask
{
public:
	//����Э��
	virtual void Parse(std::string type,std::string msg){}

	//�ظ�������Ϣ
	void ResCMD(std::string msg);

	//���ͽ������ӵ�����ͨ��
	void Send(std::string data);
	void Send(const char* data, int datasize);

	void Close();

	//��������ͨ��
	void ConnectPORT();

	virtual void Read(struct bufferevent* bev){}
	virtual void Write(struct bufferevent* bev) {}
	virtual void Event(struct bufferevent* bev,short what) {}

	void SetCallback(struct bufferevent* bev);
	bool Init() { return true; }



	std::string curDir = "/";
	std::string rootDir = ".";

	//PORT����ͨ����IP�Ͷ˿�
	std::string ip = "";
	int port = 0;

	//����ͨ��
	XFtpTask* cmdTask = 0;
protected:
	static void ReadCB(bufferevent* bev, void*  arg);
	static void EventCB(struct bufferevent *bev, short what, void* arg);
	static void WriteCB(bufferevent* bev, void*  arg);

	//bev
	struct bufferevent* bev = 0;
	FILE * fp = 0;

};

