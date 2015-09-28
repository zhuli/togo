#Togo a simple memory tool for Server

功能：<br>
1. 提供计数器模块<br>
2. 提供队列模块<br>
3. 内存锁模块<br>

*计数器模块协议：<br>
1 计数器加上某个数字，默认+1<br>
```c
COUNTER PLUS #NAME #NUM\r\n
COUNTER PLUS test 1\r\n
```
2 计数器减去某个数字，默认-1<br>
```c
COUNTER MINUS #NAME #NUM\r\n
COUNTER MINUS test 1\r\n
```
3 计数器获取一个值<br>
```c
COUNTER GET #NAME\r\n
COUNTER GET test\r\n
```
4 计数器初始化<br>
```c
COUNTER RESET #NAME\r\n
COUNTER RESET test\r\n
```

*队列模块协议：<br>
1 从左边插入一个记录<br>
```c
QUEUE LPUSH #NAME #VALUE #PRIORITY(1,2,3 优先级)\r\n
QUEUE LPUSH test 1234\r\n
QUEUE LPUSH test 1234 2\r\n
```
2 从右边插入一个记录<br>
```c
QUEUE RPUSH #NAME #VALUE #PRIORITY(1,2,3 优先级)\r\n
QUEUE RPUSH test 1234\r\n
QUEUE RPUSH test 1234 2\r\n
```
3 从左边获取一个记录<br>
```c
QUEUE LPOP #NAME\r\n
QUEUE LPOP test\r\n
```
4 从右边获取一个记录<br>
```c
QUEUE RPOP #NAME\r\n 
QUEUE RPOP test\r\n
```
5 获取一个队列的总记录数<br>
```c
QUEUE COUNT #NAME\r\n 
QUEUE COUNT test\r\n
```
6 获取一个队列的状态<br>
```c
QUEUE STATUS #NAME\r\n
QUEUE STATUS test\r\n
```

*内存锁模块协议：<br>
1 LOCK操作<br>
```c
LOCK LOCK #NAME\r\n
LOCK LOCK test\r\n
```
2 UNLOCK操作<br>
```c
LOCK UNLOCK #NAME\r\n
LOCK UNLOCK test\r\n
```
3 获取一把锁的状态<br>
```c
LOCK STATUS #NAME\r\n
```

*返回值协议：<br>
1 操作成功<br>
```c
TOGO_STOGO_OKTOGO_E\r\n
TOGO_S#value#TOGO_E\r\n
```
2 操作失败<br>
```c
TOGO_STOGO_FAILTOGO_E\r\n
```
3 返回为空<br>
```c
TOGO_STOGO_NULLTOGO_E\r\n
```
4 命令行太长<br>
```c
TOGO_STOGO_COMMAND_TOO_BIGTOGO_E\r\n
```
5 需要发送/接收的内容太大<br>
```c
TOGO_STOO_BIGTOGO_E\r\n
```