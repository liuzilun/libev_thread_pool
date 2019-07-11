#pragma once
#include<vector>
//头文件尽量不要引用头文件
//头文件不要用命名空间 ，用std::引用
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
	//分发线程
	void Dispatch(XTask* task);
private:
	//线程数量
	int threadCount = 0;
	int lastThread = -1;
	//线程池线程
	std::vector<XThread*> threads;
	XThreadPool() {};
};

