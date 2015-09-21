#Togo a simple memory tool

功能：\  
1. 提供计数器模块\
2. 提供队列模块\  

计数器模块协议：\
1. 计数器加上某个数字，默认+1\ 
```c
COUNTER PLUS #NAME #NUM
COUNTER PLUS test 1
```
2. 计数器减去某个数字，默认-1\
```c
COUNTER MINUS #NAME #NUM
COUNTER MINUS test 1
```
3. 计数器获取一个值\
```c
COUNTER GET #NAME
COUNTER GET test
```
4. 计数器初始化\ 
```c
COUNTER RESET #NAME
COUNTER RESET test
```
