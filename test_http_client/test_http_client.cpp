#include<event2/event.h>
#include <iostream>
#include<event2/listener.h>
#include<string.h>
#include<event2/http.h>
#include<event2/bufferevent.h>
#include<event2/buffer.h>
#include<string>
#ifndef _WIN32
#include<signal.h>
#endif

using namespace std;
#define SPORT 5001

void http_client_cb(struct evhttp_request *req, void * arg)
{
	cout << "http_client_cb" << endl;
	event_base * base = (event_base *)arg;
	//服务响应错误
	if (NULL == req)
	{
		int errcode = EVUTIL_SOCKET_ERROR();
		evutil_socket_error_to_string(errcode);
		return;
	}

	//获取path 
	const char* path = evhttp_request_get_uri(req);
	cout << "request path is " << path << endl;
	string filepath = ".";
	filepath += path;

	cout << "file path is " << filepath << endl;
	//如果跟径中有目录，需要创建目录，此时暂不做目录处理

	FILE* fp = fopen(filepath.c_str(), "wb");
	if (!fp)
	{
		cout << "open file " << filepath << "failed! " << endl;
	}


	// 获取返回的code 200   404 ..
	cout << "Response: " << evhttp_request_get_response_code(req); //200
	cout << " "<<evhttp_request_get_response_code_line(req) << endl;    //OK

	char buf[1024] = { 0 };
	evbuffer* input = evhttp_request_get_input_buffer(req);
	for (;;)
	{
		int len = evbuffer_remove(input,buf,sizeof(buf)-1);
		if (len <= 0) break;
		buf[len] = 0;
		if (!fp)continue;
		fwrite(buf, 1, len, fp);
	}
	if (fp)
	{
		fclose(fp);
	}
	event_base_loopbreak(base);
}
int TestGetHttp()
{
	//创建event上下文
	event_base * base = event_base_new();
	if (base)
	{
		cout << "event_base_new success" << endl;
	}
	//生成请求信息　ＧＥＴ
	string http_url = "http://ffmpeg.club/index.html?id=1";
	http_url = "http://ffmpeg.club/101.jpg";

	// 分析ｕｒｌ地址
	//uri
	evhttp_uri* uri = evhttp_uri_parse(http_url.c_str());
	const char* scheme = evhttp_uri_get_scheme(uri);
	if (!scheme)
	{
		cerr << "scheme is null" << endl;
		return -1;
	}
	cout << "scheme is :" << scheme << endl;
	//host 主机
	const char* host = evhttp_uri_get_host(uri);
	if (!host)
	{
		cerr << "host is null" << endl;
		return -1;
	}
	cout << "host is :" << host << endl;
	int port = evhttp_uri_get_port(uri);
	if (port < 0)
	{
		if (strcmp(scheme, "http") == 0)
			port = 80;
	}
	cout << "port is :" << port << endl;

	const char* path = evhttp_uri_get_path(uri);
	if (!path || strlen(path) == 0)
	{
		path = "/";

	}
	if (path)
		cout << "path is " << path << endl;

	//取出问号后面的内容
	const char* query = evhttp_uri_get_query(uri);
	if (query)
	{
		cout << "query is " << query << endl;
	}
	else
	{
		cout << "query is NULL" << endl;
	}
	//连接操作 bufferevent 连接http服务器
	bufferevent* bev =
		bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

	evhttp_connection* evcon =
		evhttp_connection_base_bufferevent_new(base, 0, bev, host, port);

	//http client 请求 回调函数设置
	evhttp_request* req = evhttp_request_new(http_client_cb, base);

	//设置请求head消息报头信息
	evkeyvalq* output_headers = evhttp_request_get_output_headers(req);
	evhttp_add_header(output_headers, "Host", host);

	//发起请求
	evhttp_make_request(evcon, req, EVHTTP_REQ_GET, path);



	//事件分发处理
	event_base_dispatch(base);
	if (base)
	{
		event_base_free(base);
	}
}

int TestPostHttp()
{
	//创建event上下文
	event_base * base = event_base_new();
	if (base)
	{
		cout << "event_base_new success" << endl;
	}
	//生成请求信息　POST
	string http_url = "http://127.0.0.1:8080/index.html";
	// 分析ｕｒｌ地址
	//uri
	evhttp_uri* uri = evhttp_uri_parse(http_url.c_str());
	const char* scheme = evhttp_uri_get_scheme(uri);
	if (!scheme)
	{
		cerr << "scheme is null" << endl;
		return -1;
	}
	cout << "scheme is :" << scheme << endl;
	//host 主机
	const char* host = evhttp_uri_get_host(uri);
	if (!host)
	{
		cerr << "host is null" << endl;
		return -1;
	}
	cout << "host is :" << host << endl;
	int port = evhttp_uri_get_port(uri);
	if (port < 0)
	{
		if (strcmp(scheme, "http") == 0)
			port = 80;
	}
	cout << "port is :" << port << endl;

	const char* path = evhttp_uri_get_path(uri);
	if (!path || strlen(path) == 0)
	{
		path = "/";

	}
	if (path)
		cout << "path is " << path << endl;

	//取出问号后面的内容
	const char* query = evhttp_uri_get_query(uri);
	if (query)
	{
		cout << "query is " << query << endl;
	}
	else
	{
		cout << "query is NULL" << endl;
	}
	//连接操作 bufferevent 连接http服务器
	bufferevent* bev =
		bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);

	evhttp_connection* evcon =
		evhttp_connection_base_bufferevent_new(base, 0, bev, host, port);

	//http client 请求 回调函数设置
	evhttp_request* req = evhttp_request_new(http_client_cb, base);

	//设置请求head消息报头信息
	evkeyvalq* output_headers = evhttp_request_get_output_headers(req);
	evhttp_add_header(output_headers, "Host", host);


	//发送POSt数据
	evbuffer* output = evhttp_request_get_output_buffer(req);
	evbuffer_add_printf(output, "xcj=%d&b=%d", 1, 2);

	//发起请求
	evhttp_make_request(evcon, req, EVHTTP_REQ_POST, path);



	//事件分发处理
	event_base_dispatch(base);
	if (base)
	{
		event_base_free(base);
	}
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
    std::cout << "Test Http Client!\n"; 
	TestGetHttp();
	TestPostHttp();

#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}

