#pragma once
#include"XTask.h"
#include"XFtpTask.h"
#include<map>
class XFtpServerCMD:public XFtpTask
{
public:
	//��ʼ��
	virtual bool Init();
	virtual void Read(struct bufferevent* bev);
	virtual void Event(struct bufferevent* bev, short what);

	//ע����������,����Ҫ�����̰߳�ȫ��ע���ȣ�����ʱ��δ�ַ����߳�
	void Reg(std::string, XFtpTask* call);

	XFtpServerCMD();
	~XFtpServerCMD();

private:
	//ע��Ķ���
	std::map<std::string, XFtpTask*>calls;

	//�������ռ�����
	std::map<XFtpTask*, int>calls_del;

};

