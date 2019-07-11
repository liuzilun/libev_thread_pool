#pragma once
#include<vector>
//ͷ�ļ�������Ҫ����ͷ�ļ�
//ͷ�ļ���Ҫ�������ռ� ����std::����
class XThread;
class XTask;
class XThreadPool
{
public:
	static XThreadPool* Get()
	{
		static XThreadPool p;
		return &p;
	}

	void Init(int threadCount);
	//�ַ��߳�
	void Dispatch(XTask* task);
private:
	//�߳�����
	int threadCount = 0;
	int lastThread = -1;
	//�̳߳��߳�
	std::vector<XThread*> threads;
	XThreadPool() {};
};

