#include <iostream>
#include <vector>
#include <sw/redis++/redis++.h>
#include <chrono>
#include <thread>
#include "util.hpp"
using namespace std;
using sw::redis::Redis;

// get set
void test1(Redis &redis)
{
    cout << "1. get 和 set 使用" << endl;
    // 清空数据库，避免之前的内容干扰
    redis.flushall();

    redis.set("key1", "111");
    redis.set("key2", "222");
    redis.set("key3", "333");

    auto value1 = redis.get("key1");
    auto value2 = redis.get("key2");
    auto value3 = redis.get("key3");
    auto value4 = redis.get("key4");
    // if (value1)
    //     cout << "value1 = " << value1.value() << endl;
    // if (value2)
    //     cout << "value2 = " << value2.value() << endl;
    // if (value3)
    //     cout << "value3 = " << value3.value() << endl;
    // if (value4)
    //     cout << "value4 = " << value4.value() << endl;

    cout << "value1 = " << value1.value_or("(nil)") << endl;
    cout << "value2 = " << value2.value_or("(nil)") << endl;
    cout << "value3 = " << value3.value_or("(nil)") << endl;
    cout << "value4 = " << value4.value_or("(nil)") << endl;
}

// exists
void test2(Redis &redis)
{
    cout << "2. exists 的使用" << endl;
    redis.flushall();
    redis.set("key", "111");
    auto ret = redis.exists("key");
    cout << ret << endl;
    ret = redis.exists("key2");
    cout << ret << endl;
    cout << "多个 key：" << endl;
    redis.set("key2", "222");
    ret = redis.exists({"key", "key2", "key3"});
    cout << ret << endl;
}

// del
void test3(Redis &redis)
{
    cout << "3. del 的使用" << endl;
    redis.flushall();
    redis.set("key1", "11");
    redis.set("key2", "22");
    redis.set("key3", "33");

    auto ret = redis.del({"key1", "key2", "key4"});
    cout << ret << endl;
}

// keys
void test4(Redis &redis)
{
    cout << "4. keys 的使用" << endl;
    redis.flushall();
    redis.set("key1", "111");
    redis.set("key2", "222");
    redis.set("key3", "333");
    redis.set("key4", "444");
    redis.set("key5", "555");
    redis.set("key6", "666");
    vector<string> res;
    auto it = std::back_insert_iterator(res);
    redis.keys("*", it);
    printContainer(res);
}

// expire
void test5(Redis &redis)
{
    cout << "5. expire and ttl" << endl;
    redis.flushall();
    redis.set("key1", "111");
    redis.expire("key1", chrono::seconds(10));
    // linux sleep(s)
    // windows Sleep(ms)
    // 两者不一样，最好还是使用标准库的函数
    std::this_thread::sleep_for(3s); // 此处的 3s 是字面值常量，毫秒就是 3ms

    auto time = redis.ttl("key1");
    cout << time << endl;
}

// type
void test6(Redis &redis)
{
    cout << "6. type 的使用" << endl;
    redis.flushall();
    redis.set("key1", "111");
    string res = redis.type("key");
    cout << "key1:" << res << endl;

    redis.lpush("key2", "222");
    res = redis.type("key2");
    cout << "key2:" << res << endl;

    redis.hset("key3", "ahwei", "male");
    res = redis.type("key3");
    cout << "key3:" << res << endl;

    redis.sadd("key4", "ahwei");
    res = redis.type("key4");
    cout << "key4:" << res << endl;

    redis.zadd("key5", "ahwei", 99);
    res = redis.type("key5");
    cout << "key5:" << res << endl;
}

int main()
{
    Redis redis("tcp://127.0.0.1:6379");
    test1(redis);
    test2(redis);
    test3(redis);
    test4(redis);
    test5(redis);
    test6(redis);
    return 0;
}