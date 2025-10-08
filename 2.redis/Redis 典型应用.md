# 一、缓存

# 二、分布式锁

## 2.1 分布式锁是什么

在分布式系统中，也会涉及到多个节点访问同一个公共资源的情况，需要用**锁**来做互斥控制，避免出现类似于线程安全的问题。

> 锁：保证程序在任意执行顺序下，执行逻辑都是正确的

此时需要用到分布式锁，分布式锁本质上是一个公共的服务器，来记录加锁的状态。这个公共服务器可以是 `Redis`，也可以是其他组件（比如 `Mysql` 或者 `ZooKeeper` 等），还可以是我们自己写的一个服务。

## 2.2 分布式锁的基础实现

**本质上通过一个键值对来标识锁的状态**

我们考虑一个买票系统，多个服务器节点都需要处理这个买票逻辑：先查询指定车次的余票，如果余票 > 0，则设置余票值 -1。

```mermaid
graph LR
    subgraph 外网
        C1[客户端]
        C2[客户端]
        C3[客户端]
        C4[客户端]
        C5[客户端]
    end

    subgraph 机房
        B1[买票服务器1]
        B2[买票服务器2]
        B3[买票服务器3]

        DB[数据库]
        DB_Table["车次余票<br>001 100<br>002 150<br>003 200"]

        Redis["Redis<br>key:001, value:1"]
    end

    C1 --> B1
    C2 --> B2
    C3 --> B3
    C4 --> B1
    C5 --> B2

    B1 --> DB
    B2 --> DB
    B3 --> DB

    B1 --> Redis
    B2 --> Redis
    B3 --> Redis

```

此时，如果买票服务器1尝试买票，就需要先访问 `Redis`，在`Redis` 上设置一个键值对，比如 `key` 是车次，`value` 随便设个值。这个操作设置成功视为加锁成功，买票过程结束后再把这个键值对删掉。在上述持有锁的过程中，如果服务器2也想买票，则也会尝试给 `Redis` 上写一个键值对，`key` 同样是车次，但此时发现该车次的 `key` 已经存在了，则服务器 2 需要阻塞或者放弃。

`Redis` 中提供了 `setnx` 操作，刚好适合这个场景，`key` 不存在就设置，存在则直接失败。解锁使用 `del` 命令。

> 上述的买票场景，使用 `Mysql` 的事务也可以，但是在分布式系统中要访问的共享资源不一定是 `Mysql`，也可能是其他没有事务的存储介质，也可能是执行一段特定的操作，通过统一的服务器来完成执行动作

## 2.3 引入过期时间

在服务器加锁后，处理买票的过程中，如果服务器1意外宕机了就会导致解锁操作（删除改 `key`）不能执行，就可能引起其他服务器始终无法获取到锁的情况。

> 这个问题无法像进程内锁的解决思路那样，用 RAII 解决，因为服务器进程异常， redis 服务器还是好的，不会像 RAII 那样，锁随着进程一起消亡。

所以使用 `set ex nx` 的方式，在设置锁的同时把过期时间也设置进去。

> 注意此处必须用一条命令来执行，如果分开操作，比如 setnx 之后，再来一个单独的 expire，由于 redis 的多个指令之间不存在关联，并且即使使用了事务也无法保证这两个操作一定都成功，即无法保证原子性，所以仍然会出现无法正确释放锁的问题。

## 2.4 引入校验 id

对于 redis 中写入的加锁键值对，其他的节点也是可以删除的。

> 比如服务器写入一个 “001”：1 这样的键值对，服务器2是完全可以把 “001” 给删除掉的。
>
> 当然服务器2也不会去恶意删除，不过不能保证因为一些 bug 导致的服务器2把锁误删除的情况。

为了解决上述问题，我们可以引入一个校验 id，比如可以把设置的键值对的值，设置为服务器的编号。形如 “001”：“服务器1”

**key：针对哪个资源加锁**

**value：服务器编号**（这里假定服务器是善意的，不会乱写一个编号）

这样可以在删除 `key` 解锁的时候，先校验当前删除 `key` 的服务器是否是当初加锁的服务器，如果是，才能真正删除；不是，则不能删除。

在解锁的时候，先查询判定，再执行 `del`，这样做并非是原子的。

因为一个服务器内部也是多线程的，所以可能存在以下状况：

```mermaid
sequenceDiagram
    participant Redis
    participant Server1_ThreadA as 服务器1线程A
    participant Server1_ThreadB as 服务器1线程B
    participant Server2_ThreadC as 服务器2线程C

    Note over Redis,Server1_ThreadA: 初始状态：锁由Server1持有，value="v1"

    Server1_ThreadA->>Redis: GET lock_key  → "v1"
    Note right of Server1_ThreadA: 线程A读取到自己的锁值

    Server1_ThreadB->>Redis: GET lock_key  → "v1"
    Note right of Server1_ThreadB: 线程B也读取到相同值（仍为v1）

    Server1_ThreadA->>Redis: DEL lock_key
    Note right of Redis: 锁被删除（释放成功）

    Server2_ThreadC->>Redis: SET lock_key "v2" NX PX 30000
    Note right of Server2_ThreadC: 线程C成功加锁（新的锁v2）

    Server1_ThreadB->>Redis: DEL lock_key
    Note right of Redis: ❌ 线程B删除了线程C的锁（误删）

    Note over Redis,Server1_ThreadA: 锁被释放了两次，第二次误删他人锁！

```

| 时刻 | 操作           | 锁状态      | 说明                 |
| ---- | -------------- | ----------- | -------------------- |
| ①    | A `GET` → "v1" | 锁存在 (v1) | A 读取锁值           |
| ②    | B `GET` → "v1" | 锁仍是 (v1) | B 也读取锁值         |
| ③    | A `DEL`        | 锁被删除    | A 正常释放锁         |
| ④    | C `SET NX PX`  | 锁重建 (v2) | 另一台服务器获取新锁 |
| ⑤    | B `DEL`        | 锁被误删    | B 删除了 C 的锁 ❌    |

## 2.5 引入 Lua

为了使解锁操作有原子性，可以使用 `Redis` 的 `Lua` 脚本功能。

> Lua 语法类似于 JS，是一个动态弱类型语言。Lua 解释器一般使用 C 语言实现。Lua 语法简单精炼，执行速度快，解释器也比较轻量（Lua 解释器的可执行程序体积只有 200KB 左右）。
>
> 因此，Lua 经常作为其他程序内部嵌入的脚本语言，Redis 本身就支持 Lua 作为内嵌脚本。

使用 Lua 脚本实现上述解锁功能：

```lua
if redis.call('get', KEYS[1]) == ARGV[1] then
    return redis.call('del', KEYS[1])
else 
    return 0
end;
```

上述代码可以编写成一个 `.lua` 后缀的文件，由 `redis-cli` 或者 `redis-plus-plus` 或者 `jedis` 等客户端加载，并发送给 `redis` 服务器，由 `redis` 服务器来执行这段逻辑。

**一个 `lua` 脚本会被 `Redis` 服务器以原子的方式执行。**

## 2.6 引入看门狗

上述方案仍存在一个重要问题：当我们设置了 `key` 的过期时间之后（比如10s），可能任务还没执行完，`key` 就先过期了，这会导致锁提前失效。

设置的时间太短，可能不够，太长，可能释放锁不及时。因此相比于设置一个固定的时间，不如 **动态的调整时间** 更为合适。

所谓 `watch dog`，本质上是加锁服务器上的一个单独的线程，通过这个线程来对锁过期时间进行“续约”。**注意这个线程是业务服务器上的，不是 Redis 服务器的。**

举个例子：

初始情况下设置过期时间为 10s，同时设置看门狗线程每隔 3s 检测一次。

那么当 3s 时间到了的时候，看门狗线程就会判定当前任务是否完成：

- 如果任务已经完成，则直接通过 `lua` 脚本的方式，释放锁（删除 `key`)
- 如果任务未完成，则把过期时间重新设置为 10s （续约）

这样就不用担心锁提前失效的问题了。而且另一方面，如果该服务器挂了，看门狗线程也就随之挂了，此时无人续约，这个 `key` 自然就可以迅速过期，让其他服务器能够获取到锁了。

## 2.7 引入 Redlock 算法

实际工程中 Redis 一般是以集群方式部署的（至少是主从方式，而不是单机）。那么可能出现以下状况：

```mermaid
sequenceDiagram
    participant Server1 as 服务器1
    participant Server2 as 服务器2
    participant Master as Redis Master（旧）
    participant Slave as Redis Slave（旧）→ 新Master

    Note over Server1,Slave: 初始状态：Master和Slave正常同步

    Server1->>Master: SET lock_key "UUID1" NX EX 10
    Note right of Master: 加锁成功（主节点写入）
    Master-->>Server1: OK
    Master-->>Slave: （异步复制，未完成）

    Note over Master,Slave: ❗ Master宕机，复制尚未完成

    Note over Slave: Slave晋升为新的 Master（未含 lock_key）

    Server2->>Slave: SET lock_key "UUID2" NX EX 10
    Note right of Slave: 成功（因为锁信息丢失）
    Slave-->>Server2: OK（加锁成功）

    Note over Server1,Server2: ❌ 两个服务器都认为自己持有锁，导致并发写入问题

```

**Redlock 算法：**

```mermaid
sequenceDiagram
    participant Client as 服务器（客户端）
    participant R1 as Redis 节点1（Master）
    participant R2 as Redis 节点2（Master）
    participant R3 as Redis 节点3（Master）
    participant R4 as Redis 节点4（Master）
    participant R5 as Redis 节点5（Master）

    Note over Client,R5: RedLock 分布式锁流程（假设共5个独立Redis实例）

    Client->>R1: SET lock_key "UUID" NX PX 10s
    alt 成功
        R1-->>Client: OK
    else 超时或失败
        R1-->>Client: FAIL
    end

    Client->>R2: SET lock_key "UUID" NX PX 10s
    alt 成功
        R2-->>Client: OK
    else 超时或失败
        R2-->>Client: FAIL
    end

    Client->>R3: SET lock_key "UUID" NX PX 10s
    alt 成功
        R3-->>Client: OK
    else 超时或失败
        R3-->>Client: FAIL
    end

    Client->>R4: SET lock_key "UUID" NX PX 10s
    Client->>R5: SET lock_key "UUID" NX PX 10s

    Note over Client: 客户端统计加锁成功的节点数

    alt 成功节点 ≥ (N/2 + 1)
        Note over Client: ✅ 超过半数节点加锁成功 → 加锁成功
        Client-->>Client: 执行业务逻辑（受锁保护）
    else
        Note over Client: ❌ 未达半数 → 加锁失败，立即释放已加的锁
        Client->>R1: DEL lock_key（若存在）
        Client->>R2: DEL lock_key（若存在）
        Client->>R3: DEL lock_key（若存在）
        Client->>R4: DEL lock_key（若存在）
        Client->>R5: DEL lock_key（若存在）
    end

    Note over Client,R5: 执行完成后，客户端在所有节点上释放锁（仅删除自己UUID对应的key）

```

| 步骤                             | 机制                              | 作用                                 |
| -------------------------------- | --------------------------------- | ------------------------------------ |
| 1️⃣ 多节点独立部署                 | 每个节点都是一个独立的 Redis 实例 | 避免主从复制延迟带来的锁丢失         |
| 2️⃣ 顺序尝试加锁                   | 设置较短超时时间（如 50ms）       | 防止单节点延迟导致整体阻塞           |
| 3️⃣ 超过半数节点成功即视为加锁成功 | 满足分布式一致性原则              | 即使部分节点宕机也不影响锁的安全性   |
| 4️⃣ 未达半数立即回滚               | 删除已加成功的锁                  | 避免“半锁”状态残留                   |
| 5️⃣ 使用 UUID 验证释放             | 仅能释放自己加的锁                | 防止误删其他客户端的锁（保证安全性） |
