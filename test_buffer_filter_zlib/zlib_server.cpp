#include<event2/event.h>
#include <iostream>
#include<event2/listener.h>
#include<event2/bufferevent.h>
#include<string.h>
#include<event2/buffer.h>
#include<zlib.h>
#include<string>
#ifndef _WIN32
#include<signal.h>
#endif
using namespace std;
#define SPORT 5001
struct Status
{
	bool start = false;
	FILE* fp = 0;
	z_stream* p;
	int recvNum = 0;
	int writeNum = 0;
	~Status()
	{
		if (fp)
		{
			fclose(fp);
		}
		if (p)
		{
			inflateEnd(p);
		}
		delete p;
		p = 0;
	}
};

bufferevent_filter_result filter_in(evbuffer* s,
	evbuffer* d, ev_ssize_t limit, bufferevent_flush_mode m,void* arg)
{
	Status *status = (Status*)arg;
	cout << "server_filter_in" << endl;
	//接收客户端发送的文件名，回复OK
	if (!status->start)
	{
		char data[1024] = { 0 };
		int len = evbuffer_remove(s, data, sizeof(data) - 1);

		//cout << "server recv:" <<data<< endl;

		evbuffer_add(d, data, len);//加入到read中再处理
		return BEV_OK;
	}
	//解压
	evbuffer_iovec v_in[1];
	int n = evbuffer_peek(s, -1, NULL, v_in, 1);//读取不清理缓冲
	if (n <= 0)
	{
		return BEV_NEED_MORE;
	}
	z_stream *p = status->p;

	//zlib输入数据大小
	p->avail_in = v_in[0].iov_len;
	//zlib输入数据地址
	p->next_in = (Byte*)v_in[0].iov_base;
	//zlib申请输出空间大小
	evbuffer_iovec v_out[1];
	evbuffer_reserve_space(d, 4096, v_out, 1);

	//zlib 输出空间大小
	p->avail_out = v_out[0].iov_len;
	//zlib输出空间地址
	p->next_out = (Byte*)v_out[0].iov_base;

	//解压数据
	int re = inflate(p, Z_SYNC_FLUSH);

	if (re != Z_OK)
	{
		cerr << "deflate filed!" << endl;

	}
	//解压用了多少数据，从source evbuffer中移除
	int nread = v_in[0].iov_len - p->avail_in;

	//解压后数据大小压缩后的数据传入 des evbuffer
	int nwrite = v_out[0].iov_len - p->avail_out;

	//移除source evbuffer中数据
	evbuffer_drain(s, nread);
	//传入des evbuffer 
	v_out[0].iov_len = nwrite;
	evbuffer_commit_space(d, v_out, 1);

	cout << "servernread :" << nread << "servernwrite=" << nwrite << endl;
	status->recvNum += nread;
	status->writeNum += nwrite;
	return BEV_OK;

}
bufferevent_filter_result filter_out(evbuffer* s,
	evbuffer* d, ev_ssize_t limit, bufferevent_flush_mode m, void* arb)
{
	cout << "server_filter_out" << endl;
	return BEV_OK;
}

void read_cb(bufferevent* bev, void* arg)
{
	cout << "server_read_cb" << endl;

	Status* status = (Status*)arg;
	if (!status->start)
	{
		//001接收文件名
		char data[1024] = { 0 };
		bufferevent_read(bev, data, sizeof(data) - 1);
		// 打开写入文件
		string out = "out/";
		out += data;
		status->fp = fopen(out.c_str(), "wb");
		if (!status->fp)
		{
			cout << "server open " << out << "failed" << endl;
			return;
		}

		//回复OK
		bufferevent_write(bev, "OK", 2);
		status->start = true;
		return;
	}
	do {
		//写入文件
		char data[1024] = { 0 };
		int len = bufferevent_read(bev, data, sizeof(data));
		if (len >= 0)
		{
			fwrite(data, 1, len, status->fp);
			fflush(status->fp);
		}
 	} while (evbuffer_get_length(bufferevent_get_input(bev)) > 0);


}
void event_cb(bufferevent* bev, short events, void* arg)
{
	cout << "server_event_cb" <<events<< endl;

	Status *status = (Status*)arg;
	if (events&BEV_EVENT_EOF)
	{
		cout << "Server recv :" << status->recvNum;
		cout << "Write  " << status->writeNum << endl;
		delete status;
		//if (status->fp)
		//{
		//	fclose(status->fp);
		//	status->fp = 0;

		//}

		bufferevent_free(bev);

	}


}
void listen_cb(struct evconnlistener* e, evutil_socket_t s, struct sockaddr* a, int socklen, void* arg)
{
	cout << "server_call back!" << endl;

	event_base* base = (event_base*)arg;
	//1.创建一个bufferevent 用来通信
	bufferevent* bev = bufferevent_socket_new(base, s, BEV_OPT_CLOSE_ON_FREE);
	Status* stu = new Status();
	stu->p = new z_stream();
	inflateInit(stu->p);
	
	//2.添加输入过滤，并设置输入过滤回调
	bufferevent* bev_filter = bufferevent_filter_new(bev,
		filter_in,//输入过滤函数)
		0,//输出过滤
		BEV_OPT_CLOSE_ON_FREE,//关闭filter同时管理 bufferevent
		0,//清理回调
		stu//参数传递
	);
	//3.设置回调，读取，事件（处理连接断开，超时等）
	bufferevent_setcb(bev_filter, read_cb, 0, event_cb, stu);
	bufferevent_enable(bev_filter, EV_READ | EV_WRITE);
}
void Server(event_base* base)
{
	//监听端口socket,bind,listen，绑定事件
	sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SPORT);

	evconnlistener *ev = evconnlistener_new_bind(base,//上下文
		listen_cb,
		base,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,//地址重用，listen 关闭同时关socket
		10,//连接队列大小，对应listen函数
		(sockaddr*)&sin,  //绑定的地址和端口
		sizeof(sin)
	);

}