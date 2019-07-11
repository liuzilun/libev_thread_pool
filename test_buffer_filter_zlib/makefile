test_buffer_filter_zlib:test_buffer_filter_zlib.cpp zlib_client.cpp zlib_server.cpp
	g++ $^ -o $@ -levent -lz
	./$@
clean:
	rm -rf test_buffer_filter_zlib
	rm -rf *.o
