#include "XThreadPool.h"
#include"XThread.h"
#include<thread>
#include <iostream>
#include"XTask.h"

using namespace std;
void XThreadPool::Init(int threadCount)
{
	this->threadCount = threadCount;
	this->lastThread = -1;

	for (int i = 0; i < threadCount; i++)
	{
		XThread* t = new XThread();
		cout << "Create Thread " << i<<endl;
		
		//启动线程
		t->id = i + 1;
		t->Setup();
		t->Start();
		threads.push_back(t);// 创建，分发等操作都在主线程，此处不用考虑锁
		this_thread::sleep_for(10ms);
	}

}
//分发线程
void XThreadPool::Dispatch(XTask* task)
{
	//轮询机制
	if (!task)return;
	int tid = (lastThread + 1) % threadCount;

	lastThread = tid;
	XThread* t = threads[tid];

	t->AddTask(task);
	//激活线程
	t->Activate();

}
