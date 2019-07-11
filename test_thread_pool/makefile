test_thread_pool:test_thread_pool.cpp XFtpServerCMD.cpp XThread.cpp XThreadPool.cpp
	g++ $^ -o $@ -levent -lpthread
	./$@
clean:
	rm -rf test_thread_pool
	rm -rf *.o
