#include <iostream>
#include <string>
#include <sw/redis++/redis++.h>
using namespace std;

int main () {
    // 创建 Redis对象的时候，需要在构造函数中指定 redis 服务器的地址和端口
    sw::redis::Redis redis("tcp://127.0.0.1:6379");
    // 调用 ping 方法让客户端给服务器发一个 PING，然后服务器就会返回一个 PONG，就通过返回值获取到
    string res = redis.ping();
    // 打印 PONG
    cout << res << endl;
    return 0;
}