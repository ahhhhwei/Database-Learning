#include <iostream>
#include <vector>
#include <string>
#include <sw/redis++/redis++.h>
#include <chrono>
#include "util.hpp"

using std::cout;
using std::endl;

using sw::redis::Redis;
using namespace std::chrono_literals;

void test1(Redis &redis)
{
    cout << "get and set" << endl;
    redis.flushall();
    redis.set("key1", "111");
    redis.set("key2", "222");

    auto value1 = redis.get("key1");
    auto value2 = redis.get("key2");
    cout << "value1 = " << value1.value() << endl;
    cout << "value2 = " << value2.value() << endl;
}

void test2(Redis &redis)
{
    cout << "get and set(带超时时间)" << endl;
    redis.flushall();
    redis.set("key1", "111", std::chrono::seconds(10));

    long long time = redis.ttl("key1");
    cout << "time: " << time << endl;
}

void test3(Redis &redis)
{
    cout << "set NX and XX" << endl;
    redis.flushall();
    redis.set("key1", "111", 0s, sw::redis::UpdateType::NOT_EXIST);
    auto val = redis.get("key1");
    if (val)
    {
        cout << "value: " << val.value() << endl;
    }
    else
    {
        cout << "key 不存在" << endl;
    }
}

// mset
void test4(Redis &redis)
{
    cout << "mset" << endl;
    redis.flushall();
    // redis.mset({ std::make_pair("key1", "111"), std::make_pair("key2", "222"), std::make_pair("key3", "333")});
    // 也可以用 vector<std::pair<string, string>>
    std::vector<std::pair<std::string, std::string>> keys = {
        {"key1", "111"},
        {"key2", "222"},
        {"key3", "333"},
    };
    redis.mset(keys.begin(), keys.end());
    auto value1 = redis.get("key1");
    auto value2 = redis.get("key2");
    auto value3 = redis.get("key3");
    cout << "value1 = " << value1.value() << endl;
    cout << "value2 = " << value2.value() << endl;
    cout << "value3 = " << value3.value() << endl;
}

// mget
void test5(Redis &redis)
{
    cout << "mget" << endl;
    redis.flushall();
    std::vector<std::pair<std::string, std::string>> keys = {
        {"key1", "111"},
        {"key2", "222"},
        {"key3", "333"},
    };
    redis.mset(keys.begin(), keys.end());
    std::vector<sw::redis::OptionalString> result;
    auto it = std::back_insert_iterator(result);
    redis.mget({"key1", "key2", "key3", "key4"}, it);
    printContainerOptional(result);
}

// getrange 和 setrange
void test6(Redis &redis)
{
    cout << "getrange and setrange" << endl;
    redis.flushall();
    redis.set("key", "abcdefghijk");
    std::string result = redis.getrange("key", 2, 5);
    cout << "result: " << result << endl;
    redis.setrange("key", 2, "xyz");
    auto value = redis.get("key");
    cout << "value: " << value.value() << endl;
}

// incr 和 decr
void test7(Redis &redis)
{
    cout << "incr and decr" << endl;
    redis.flushall();
    redis.set("key", "100");
    cout << "incr: " << endl;
    long long result = redis.incr("key");
    cout << "result: " << result << endl;
    auto value = redis.get("key");
    cout << "value: " << value.value() << endl;

    cout << "decr: " << endl;
    result = redis.decr("key");
    cout << "result: " << result << endl;
    value = redis.get("key");
    cout << "value: " << value.value() << endl;
}
int main()
{
    Redis redis("tcp://127.0.0.1:6379");
    // test1(redis);
    // test2(redis);
    // test3(redis);
    // test4(redis);
    // test5(redis);
    // test6(redis);
    test7(redis);

    return 0;
}