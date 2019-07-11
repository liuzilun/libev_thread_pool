#include<event2/event.h>
#include<event2/thread.h>
#include<event2/listener.h>
#include<event2/bufferevent.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<string>
#include <iostream>
#define SPORT 5001
#ifndef _WIN32
#include<signal.h>
#endif // !

using namespace std;

static string recvstr = "";
static int recvCount = 0;
static int sendCount = 0;
void read_cb(bufferevent* be, void* arg)
{
	cout << "read" << endl;
	char data[1024] = { 0 };
	//读取输入缓冲数据
	int len = bufferevent_read(be, data, sizeof(data) - 1);
	cout << data << flush;
	if (len <= 0)return;
	recvstr += data;
	recvCount+=len;



	//发送数据，写入到输出缓冲
	bufferevent_write(be, "OK", 3);

}
void write_cb(bufferevent* be, void* arg)
{
	//cout << "write" << endl;
}
//错误，超时，连接断开会进入
void event_cb(bufferevent* be,short events, void* arg)
{
	cout << "event" << endl;
	//超时事件发生后，数据读取停止
	if (events & BEV_EVENT_TIMEOUT&&events&BEV_EVENT_READING)
	{
	//cout << "BEV_EVENT_TIMEOUT&&BEV_EVENT_READING" << endl;
	     
	    //bufferevent_enable(be, EV_READ);
	    char da[1024] = {0};
	    //读取输入缓冲数据
	    int len = bufferevent_read(be, da, sizeof(da) - 1);

        if(len>0)
		{
			recvCount+=len;
			recvstr+=da;
		}
		
		bufferevent_free(be);//超时一般断开连接
		
		cout<<recvstr<<endl;
		
		cout<<"recvcount = "<<recvCount<<endl;
		cout<<"sendcount = "<<sendCount<<endl;
	}
	else if (events&BEV_EVENT_ERROR)
	{
		bufferevent_free(be);
	}
	else
	{
		cout << "OTHERS" << endl;
	}
}

void cli_read_cb(bufferevent* be, void* arg)
{
	cout << "cread" << endl;
	char data[1024] = { 0 };
	//读取输入缓冲数据
	int len = bufferevent_read(be, data, sizeof(data) - 1);
	cout << "[" << data << "]" << endl;
	if (len <= 0)return;


	//发送数据，写入到输出缓冲
	bufferevent_write(be, "OK", 3);

}
void cli_write_cb(bufferevent* be, void* arg)
{
	cout << "cli_write" << endl;
	FILE* fp = (FILE*)arg;
	char data[1024] = {0};
	int len = fread(data,1,sizeof(data)-1,fp);
	if(len<=0)
	{
		//到文件尾或是出错
		fclose(fp);
		//立刻清理可能会导致缓冲数据没有发送结束
		//bufferevent_free(be);
		bufferevent_disable(be,EV_WRITE);
		return;
	}
	else
	{
		sendCount+=len;
		//写入buffer
		bufferevent_write(be,data,len);
	}
}
//错误，超时，连接断开会进入
void cli_event_cb(bufferevent* be, short events, void* arg)
{
	cout << "cli_event" << endl;
	//超时事件发生后，数据读取停止
	if (events & BEV_EVENT_TIMEOUT&&events&BEV_EVENT_READING)
	{
    	cout << "BEV_EVENT_TIMEOUT&&BEV_EVENT_READING" << endl;


		//bufferevent_enable(be, EV_READ);
		bufferevent_free(be);//超时一般断开连接
		return;
	}
	else if (events&BEV_EVENT_ERROR)
	{
		bufferevent_free(be);
		return;
	}
	
	if(events&BEV_EVENT_EOF)
	{
		cout<<"BEV_EVENT_EOF"<<endl;
		bufferevent_free(be);
		
	}
	if(events&BEV_EVENT_CONNECTED)
	{
		cout<<"BEV_EVENT_CONNECTED"<<endl;
		//触发write
		bufferevent_trigger(be,EV_WRITE,0);
		
	}

}




void listen_cb(evconnlistener* ev, evutil_socket_t s, sockaddr* sin, int slen, void* arg)
{
	cout << "listen call back" << endl;
	event_base* base = (event_base*)arg;
	//BEV_OPT_CLOSE_ON_FREE 清理BUffevent时关闭socket
	bufferevent *bev = bufferevent_socket_new(base,s,BEV_OPT_CLOSE_ON_FREE);
	bufferevent_enable(bev,EV_READ | EV_WRITE);

	//设置水位
	//读取水位
	bufferevent_setwatermark(bev, EV_READ,
		10,  //低水位    0无限
		0);   //高水位  0无限

	bufferevent_setwatermark(bev, EV_WRITE,
		10,  //低水位    0无限
		0);   //高水位  0无限

	// 超时时间设置
	timeval t1 = { 1,0 };
	bufferevent_set_timeouts(bev, &t1, 0);

	//设置回调函数
	bufferevent_setcb(bev, read_cb, write_cb, event_cb, base);

}
int main()
{


#ifdef _WIN32
	//初始化 socket
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);
#else
	//忽略管道信号，发送数据给已关闭的socket
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		return 1;
#endif

	   event_base* base = event_base_new();
	   //创建网络服务器
	   sockaddr_in sin;
	   memset(&sin, 0, sizeof(sin));

	   sin.sin_family = AF_INET;
	   sin.sin_port = htons(5001);

	   evconnlistener* ev = evconnlistener_new_bind(base, listen_cb, base, LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
		   10,//listen back
		   (sockaddr*)&sin,
		   sizeof(sin));
	   {
		   
		   
		   
		   //调用客户端代码
		   //-1内部创建socket 
		   bufferevent* bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
		   if (!bev)
		   {
			   cout << "failed !" << endl;
		   }
		   sockaddr_in sin;
		   memset(&sin, 0, sizeof(sin));
		   sin.sin_family = AF_INET;
		   sin.sin_port = htons(5001);
		   evutil_inet_pton(AF_INET, "192.168.226.129", &sin.sin_addr.s_addr);

		   
		   FILE* fp = fopen("test_buffer_client.cpp","rb");
 

		   
		   //y设置回调函数
		   bufferevent_setcb(bev, cli_read_cb, cli_write_cb, cli_event_cb, fp);
		   bufferevent_enable(bev, EV_READ | EV_WRITE);
		   int re = bufferevent_socket_connect(bev, (sockaddr*)&sin, sizeof(sin));
		   if (re == 0)
		   {
			   cout << "connetcted" << endl;
		   }
	   }
		event_base_dispatch(base);
		
		event_base_free(base);
		evconnlistener_free(ev);
	return 0;
}

