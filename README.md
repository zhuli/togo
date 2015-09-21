#Togo a simple memory tool

功能：<br>
1. 提供计数器模块<br>
2. 提供队列模块<br>

>计数器模块协议：<br>
>>计数器加上某个数字，默认+1<br>
```c
COUNTER PLUS #NAME #NUM
COUNTER PLUS test 1
```
>>计数器减去某个数字，默认-1<br>
```c
COUNTER MINUS #NAME #NUM
COUNTER MINUS test 1
```
>>计数器获取一个值<br>
```c
COUNTER GET #NAME
COUNTER GET test
```
>>计数器初始化<br>
```c
COUNTER RESET #NAME
COUNTER RESET test
```
