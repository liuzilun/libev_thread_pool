#include<event2/event.h>
#include <iostream>
#include<event2/listener.h>
#include<event2/bufferevent.h>
#include<string.h>
#include<zlib.h>
#include<event2/buffer.h>
#ifndef _WIN32
#include<signal.h>
#endif

constexpr auto FILEPATH = "001.bmp";
using namespace std;
struct ClientStatus
{
	FILE* fp = 0;
	bool fileend = false;
	bool startSend = false;
	z_stream* z_output = 0;
	int readNum = 0;
	int sendNum = 0;
	~ClientStatus()
	{
		z_output = 0;
		if (fp)
		{
			fclose(fp);
			fp = 0;
		}
		if (z_output)
		{
			deflateEnd(z_output);
		}
		delete z_output;
	}
};

bufferevent_filter_result cli_filter_out(evbuffer* s,
	evbuffer* d, ev_ssize_t limit, bufferevent_flush_mode m, void* arg)
{
	cout << "cli_filter_out" << endl;
	ClientStatus* sta = (ClientStatus*)arg;
	if (!sta->startSend)
	{
		//发送文件名消息001去掉
		char data[1024] = { 0 };
		int len = evbuffer_remove(s, data, sizeof(data) );
		evbuffer_add(d, data, len);
		return BEV_OK;
	}
	//	//压缩文件
	//取出buffer中的数据的引用
	evbuffer_iovec v_in[1];
	int n = evbuffer_peek(s, -1, 0, v_in, 1);
	if (n <= 0)
	{
		if (sta->fileend)
		{
			return BEV_OK;
		}
		//没有数据；不会进入写入回调
		return BEV_NEED_MORE;
	}
	//zlib上下文}
	z_stream *p = sta->z_output;
	if (!p)
	{
		return BEV_ERROR;
	}
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

	//zlib压缩
	int re = deflate(p, Z_SYNC_FLUSH);
	if (re != Z_OK)
	{
		cerr << "deflate filed!" << endl;

	}
	//压缩用了多少数据，从source evbuffer中移除
	int nread = v_in[0].iov_len - p->avail_in;

	//压缩后数据大小压缩后的数据传入 des evbuffer
	int nwrite = v_out[0].iov_len - p->avail_out;

	//移除source evbuffer中数据
	evbuffer_drain(s, nread);
	//传入des evbuffer 
	v_out[0].iov_len = nwrite;
	evbuffer_commit_space(d, v_out, 1);
	
	cout << "nread :" << nread << "nwrite=" << nwrite << endl;
	sta->readNum += nread;
	sta->sendNum += nwrite;
	return BEV_OK;
}
void cli_read_cb(bufferevent* bev, void* arg)
{
	cout << "cli_read_cb recv " << endl;
	ClientStatus* sta = (ClientStatus*)arg;
	//002接收到服务端的回复
	char data[1024] = { 0 };
	int len = bufferevent_read(bev, data, sizeof(data) - 1);
	if (strcmp(data, "OK") == 0)
	{
		cout << data << endl;
		sta->startSend = true;
		//开始发送文件 触发write回调
		bufferevent_trigger(bev, EV_WRITE, 0);
	}
	else
	{
		bufferevent_free(bev);
	}
	cout << "cli_read_cb len :" << len << endl;
}
void cli_write_cb(bufferevent* bev, void* arg)
{
	ClientStatus* status = (ClientStatus*)arg;
	//发送文件
	FILE* fp = status->fp;
	//判断什么清理资源 
	if (status->fileend)
	{
		//判断缓冲是否有数据，如果有就刷新
		//获取非过滤器绑定的buffer
		//获取输出缓冲及大小
		bufferevent* be = bufferevent_get_underlying(bev);
		evbuffer* evb = bufferevent_get_output(be);
		int len = evbuffer_get_length(evb);
		if (len <= 0)
		{
			cout << "Client read " << status->readNum << "Client sends "<<status->sendNum<<endl;
			bufferevent_free(bev);//立刻清理
			delete status;
			return;
		}
		//刷 新缓冲
		bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
		return;
	}
	if (!fp)return;
	//
	cout << "cli_write_cb" << endl;
	char data[1024] = { 0 };
	//读取文件
	int len = fread(data, 1, sizeof(data), fp);
	if (len <= 0)
	{
		status->fileend = true;
		bufferevent_flush(bev, EV_WRITE, BEV_FINISHED);
		return;
	}
	bufferevent_write(bev, data, len);
}
void cli_event_cb(bufferevent* be, short events, void* arg)
{
	cout << "client_event_cb:events = " << events << endl;
	if (events&BEV_EVENT_CONNECTED)
	{
		cout << "BEV_EVENT_CONNECTED" << endl;
		//001.发送文件名
		bufferevent_write(be, FILEPATH, strlen(FILEPATH));

		FILE* fp = fopen(FILEPATH, "rb");
		if (!fp)
		{
			cout << "fopen failed !" << endl;
			return;
		}
		ClientStatus * s = new ClientStatus();
		s->fp = fp;

		//初始化zlib上下文
		s->z_output = new z_stream();
		deflateInit(s->z_output, Z_DEFAULT_COMPRESSION);

		//创建输入过滤
		bufferevent* bev_filter = bufferevent_filter_new(be, 0, cli_filter_out,
			BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS, 0, s);

		//设置读取写入和事件回调
		bufferevent_setcb(bev_filter,
			cli_read_cb, cli_write_cb,
			cli_event_cb, s);
		//设置权限
		bufferevent_enable(bev_filter, EV_READ | EV_WRITE);
	}
	return;
}
void Client(event_base* base)
{
	cout << "begin Client" << endl;
	//1.连接服务端
	sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(5001);
	evutil_inet_pton(AF_INET, "127.0.0.1", &sin.sin_addr.s_addr);
	bufferevent* bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
	if (!bev)
	{
		cout << "client failed !" << endl;
		return;
	}


	//只绑定事件回调，用来确认连接成功
	bufferevent_enable(bev, EV_READ | EV_WRITE);
	bufferevent_setcb(bev, 0, 0, cli_event_cb, 0);

	bufferevent_socket_connect(bev, (sockaddr*)&sin, sizeof(sin));
	//2,发送文件名
	//3。接收回复确认OK

}