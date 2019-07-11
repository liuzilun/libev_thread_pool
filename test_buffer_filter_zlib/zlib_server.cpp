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
	//���տͻ��˷��͵��ļ������ظ�OK
	if (!status->start)
	{
		char data[1024] = { 0 };
		int len = evbuffer_remove(s, data, sizeof(data) - 1);

		//cout << "server recv:" <<data<< endl;

		evbuffer_add(d, data, len);//���뵽read���ٴ���
		return BEV_OK;
	}
	//��ѹ
	evbuffer_iovec v_in[1];
	int n = evbuffer_peek(s, -1, NULL, v_in, 1);//��ȡ��������
	if (n <= 0)
	{
		return BEV_NEED_MORE;
	}
	z_stream *p = status->p;

	//zlib�������ݴ�С
	p->avail_in = v_in[0].iov_len;
	//zlib�������ݵ�ַ
	p->next_in = (Byte*)v_in[0].iov_base;
	//zlib��������ռ��С
	evbuffer_iovec v_out[1];
	evbuffer_reserve_space(d, 4096, v_out, 1);

	//zlib ����ռ��С
	p->avail_out = v_out[0].iov_len;
	//zlib����ռ��ַ
	p->next_out = (Byte*)v_out[0].iov_base;

	//��ѹ����
	int re = inflate(p, Z_SYNC_FLUSH);

	if (re != Z_OK)
	{
		cerr << "deflate filed!" << endl;

	}
	//��ѹ���˶������ݣ���source evbuffer���Ƴ�
	int nread = v_in[0].iov_len - p->avail_in;

	//��ѹ�����ݴ�Сѹ��������ݴ��� des evbuffer
	int nwrite = v_out[0].iov_len - p->avail_out;

	//�Ƴ�source evbuffer������
	evbuffer_drain(s, nread);
	//����des evbuffer 
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
		//001�����ļ���
		char data[1024] = { 0 };
		bufferevent_read(bev, data, sizeof(data) - 1);
		// ��д���ļ�
		string out = "out/";
		out += data;
		status->fp = fopen(out.c_str(), "wb");
		if (!status->fp)
		{
			cout << "server open " << out << "failed" << endl;
			return;
		}

		//�ظ�OK
		bufferevent_write(bev, "OK", 2);
		status->start = true;
		return;
	}
	do {
		//д���ļ�
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
	//1.����һ��bufferevent ����ͨ��
	bufferevent* bev = bufferevent_socket_new(base, s, BEV_OPT_CLOSE_ON_FREE);
	Status* stu = new Status();
	stu->p = new z_stream();
	inflateInit(stu->p);
	
	//2.���������ˣ�������������˻ص�
	bufferevent* bev_filter = bufferevent_filter_new(bev,
		filter_in,//������˺���)
		0,//�������
		BEV_OPT_CLOSE_ON_FREE,//�ر�filterͬʱ���� bufferevent
		0,//����ص�
		stu//��������
	);
	//3.���ûص�����ȡ���¼����������ӶϿ�����ʱ�ȣ�
	bufferevent_setcb(bev_filter, read_cb, 0, event_cb, stu);
	bufferevent_enable(bev_filter, EV_READ | EV_WRITE);
}
void Server(event_base* base)
{
	//�����˿�socket,bind,listen�����¼�
	sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(SPORT);

	evconnlistener *ev = evconnlistener_new_bind(base,//������
		listen_cb,
		base,
		LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,//��ַ���ã�listen �ر�ͬʱ��socket
		10,//���Ӷ��д�С����Ӧlisten����
		(sockaddr*)&sin,  //�󶨵ĵ�ַ�Ͷ˿�
		sizeof(sin)
	);

}