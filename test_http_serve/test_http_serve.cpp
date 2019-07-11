#include<event2/event.h>
#include <iostream>
#include<event2/listener.h>
#include<event2/http.h>
#include<string.h>
#include<event2/keyvalq_struct.h>
#include<event2/buffer.h>
#include<string>
#ifndef _WIN32
#include<signal.h>
#endif


using namespace std;
#define SPORT 5001
#define WEBROOT "."
#define DEFAULTINDEX "index.html"
void http_cb(struct evhttp_request * rq, void *arg)
{
	cout << "http_cb!" << endl;
	//一.获取浏览器的请求信息
	//1.uri 
	const char* uri = evhttp_request_get_uri(rq);
	cout << "uri:" << uri << endl;
	//2.请求类型  GET POST
	string rqtype;
	switch (evhttp_request_get_command(rq))
	{
	case EVHTTP_REQ_GET:
		rqtype = "GET";
		break;
	case EVHTTP_REQ_POST:
		rqtype = "POST";
		break;
	}
	cout << "rqtype:" << rqtype << endl;
	//3.消息报头
	evkeyvalq * headers = evhttp_request_get_input_headers(rq);
	cout << "headers=============" << endl;
	for (evkeyval* p = headers->tqh_first; p != NULL; p = p->next.tqe_next)
	{
		cout << p->key << ":" << p->value << endl;
	}
	//4.请求正文 （GET为空，POST有表单信息）
	evbuffer* inbuf = evhttp_request_get_input_buffer(rq);
	char buf[1024] = { 0 };
	cout << "=======intput data==========" << endl;
	while (evbuffer_get_length(inbuf))
	{
		int n = evbuffer_remove(inbuf, buf, sizeof(buf) - 1);
		if (n > 0)
		{
			buf[n] = '\0';
			cout << buf << endl;
		}

	}
	//二.回复浏览器
	//1.状态行，消息报头 响应正文
	//分析出请求的文件 从uri中
	//设置根目录WEBROOT
	string filepath = WEBROOT;
	filepath += uri;
	if (strcmp(uri, "/") == 0)
	{
		//默认加上首页文件
		filepath += DEFAULTINDEX;
	}
	//消息报头
	evkeyvalq* outhead = evhttp_request_get_output_headers(rq);
	//要支持图片，js css 下载zip文件
	//获取文件的后缀名
	//./root/index.html

	int pos = filepath.rfind('.');
	string postfix = filepath.substr(pos + 1, filepath.size() - (pos + 1));
	if (postfix == "jpg" || postfix == "gif" || postfix == "png")
	{
		string tmp = "image/" + postfix;
		evhttp_add_header(outhead, "Content-Type", tmp.c_str());
	}
	else if (postfix == "zip")
	{
		evhttp_add_header(outhead, "Content-Type", "application/zip");
	}
	else if (postfix == "html")
	{
		evhttp_add_header(outhead, "Content-Type", "text/html;charset=UTF8");
	}
	else if (postfix == "css")
	{
		evhttp_add_header(outhead, "Content-Type", "text/css");
	}


	//_CRT_SECURE_NO_WARNINGS  windows 上面需要加这个宏才可以编译过
	//项目-属性-C/C++-预编译
	//读到HTML亠件返回正文
	FILE* fp = fopen(filepath.c_str(), "rb");
	if (!fp)
	{
		evhttp_send_reply(rq, HTTP_NOTFOUND, "", 0);
		return;
	}
	evbuffer* outbuf = evhttp_request_get_output_buffer(rq);
	for (;;)
	{
		int len = fread(buf, 1,sizeof(buf),fp);
		if (len <= 0)
		{
			break;
		}
		evbuffer_add(outbuf, buf, len);
	}
	fclose(fp);
	evhttp_send_reply(rq, HTTP_OK, "", outbuf);
}

int main()
{

#ifdef _WIN32
	//初始化 socket
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);
#else
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		return 1;
	}
#endif
    std::cout << "Test HttpServer!\n"; 

	//创建event上下文
	event_base * base = event_base_new();
	if (base)
	{
		cout << "event_base_new success" << endl;
	}

	//http 服务器
   //1.创建evhttp上下文
	evhttp* evh = evhttp_new(base );
	//2.绑定端口和IP
	if (evhttp_bind_socket(evh, "0.0.0.0", 8080) != 0)
	{
		cout << "bind socke failed!" << endl;
	}
	//3.设定回调函数
	evhttp_set_gencb(evh, http_cb, 0);

	//事件分发处理
	event_base_dispatch(base);
#ifdef _WIN32
	WSACleanup();
#endif
	if (base)
	{
		event_base_free(base);
	}
	if (evh)
	{
		evhttp_free(evh);
	}

	return 0;
}

