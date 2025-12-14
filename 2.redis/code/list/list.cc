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

typedef std::vector<std::string> VecStr;

void test1(Redis &redis)
{
    cout << "lpush and lrange" << endl;
    redis.flushall();

    // 插入单个元素
    redis.lpush("key", "111");

    // 插入一组元素，基于初始化列表
    redis.lpush("key", {"222", "333", "444"});

    // 插入一组元素，基于迭代器
    VecStr values = {"555", "666", "777"};
    redis.lpush("key", values.begin(), values.end());

    // lrange 获取到列表中的元素
    VecStr results;
    auto it = std::back_insert_iterator(results);
    redis.lrange("key", 0, -1, it);

    printContainer(results);
}

void test2(Redis &redis)
{
    cout << "rpush" << endl;
    redis.flushall();

    // 插入单个元素
    redis.rpush("key", "111");

    // 插入一组元素，基于初始化列表
    redis.rpush("key", {"222", "333", "444"});

    // 插入一组元素，基于迭代器
    VecStr values = {"555", "666", "777"};
    redis.rpush("key", values.begin(), values.end());

    // lrange 获取到列表中的元素
    VecStr results;
    auto it = std::back_insert_iterator(results);
    redis.lrange("key", 0, -1, it);

    printContainer(results);
}

void test3(Redis &redis)
{
    cout << "lpop and rpop" << endl;
    redis.flushall();

    redis.rpush("key", {"1", "2", "3", "4"});

    auto result = redis.lpop("key");

    if (result)
    {
        cout << "lpop: " << result.value() << endl;
    }

    result = redis.rpop("key");

    if (result)
    {
        cout << "lpop: " << result.value() << endl;
    }
}

void test4(Redis &redis)
{
    cout << "blpop" << endl;
    redis.flushall();

    auto result = redis.blpop("key");

    if (result)
    {
        cout << "key: " << result.value().first << endl;
        cout << "elem: " << result.value().second << endl;
    }

    else
    {
        cout << "result 无效" << endl;
    }
}

void test5(Redis &redis)
{
    cout << "llen" << endl;
    redis.flushall();

    redis.rpush("key", {"1", "22", "333", "4444"});
    long long len = redis.llen("key");
    cout << "len = " << len << endl;
}
int main()
{
    Redis redis("tcp://127.0.0.1:6379");
    // test1(redis);
    // test2(redis);
    // test3(redis);
    // test4(redis);
    test5(redis);

    return 0;
}