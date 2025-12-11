hello:hello.cc
	g++ -std=c++17 -o $@ $^ /usr/local/lib64/libredis++.a /usr/local/lib/libhiredis.a -pthread

.PHONY:clean
clean:
	rm hello
