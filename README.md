#Togo a simple memory tool

功能：<br>
1. 提供计数器模块<br>
2. 提供队列模块<br>

*计数器模块协议：<br>
1 计数器加上某个数字，默认+1<br>
```c
COUNTER PLUS #NAME #NUM
COUNTER PLUS test 1
```
2 计数器减去某个数字，默认-1<br>
```c
COUNTER MINUS #NAME #NUM
COUNTER MINUS test 1
```
3 计数器获取一个值<br>
```c
COUNTER GET #NAME
COUNTER GET test
```
4 计数器初始化<br>
```c
COUNTER RESET #NAME
COUNTER RESET test
```

*队列模块协议：<br>
1 从左边插入一个记录<br>
```c
QUEUE LPUSH #NAME #VALUE #PRIORITY(1,2,3 优先级)
QUEUE LPUSH test 1234
QUEUE LPUSH test 1234 2
```
2 从右边插入一个记录<br>
```c
QUEUE RPUSH #NAME #VALUE #PRIORITY(1,2,3 优先级)
QUEUE RPUSH test 1234
QUEUE RPUSH test 1234 2
```
3 从左边获取一个记录<br>
```c
QUEUE LPOP #NAME 
QUEUE LPOP test
```
4 从右边获取一个记录<br>
```c
QUEUE RPOP #NAME 
QUEUE RPOP test
```
5 获取一个队列的总记录数<br>
```c
QUEUE COUNT #NAME 
QUEUE COUNT test
```
6 获取一个队列的状态<br>
```c
QUEUE STATUS #NAME 
QUEUE STATUS test
```
