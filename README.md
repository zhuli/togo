#Togo a simple memory tool for Server
#下载地址：
Togo 源码包：https://github.com/zhuli/togo/raw/master/version/togo-1.0.tar.gz<br/>
Togo_PHP扩展 ： https://github.com/SKPHP1989/php_togo/archive/master.zip

#安装说明：

```
#安装libevent2.0.22
wget https://sourceforge.net/projects/levent/files/libevent/libevent-2.0/libevent-2.0.22-stable.tar.gz
chmod -R 755 libevent-2.0.22-stable.tar.gz
tar -zvxf libevent-2.0.22-stable.tar.gz
cd libevent-2.0.22-stable
./configure --prefix=/usr/
make
make install

#安装togo
wget https://github.com/zhuli/togo/raw/master/version/togo-1.1.tar.gz
tar -zvxf togo-1.1.tar.gz
cd togo-1.1
./configure --prefix=/usr/local/togo
make
make install

#如果出现./togo: error while loading shared libraries: libevent-2.0.so.5的错误
#libevent-2.0.so.5拷贝到或者/lib64/（lib64只有在centos中会有）
cp /usr/lib/libevent-2.0.so.5 /usr/lib64/

#运行togo
/usr/local/togo/bin/togo -p 8787 -c /usr/local/togo/conf/togo.conf -l /var/log/togo.log

```

#功能说明：
1. 获取版本号和退出<br>
2. 提供计数器模块<br>
3. 提供队列模块<br>
4. 内存锁模块<br>
5. 缓存模块<br/>

##1. 获取版本号和退出：
###1. 获取TOGO版本
```
VERSION\r\n
```
###2. 断开连接：<br>
```
QUIT\r\n
```

##2. 计数器模块协议
###1. 计数器加上某个数字，默认+1
```
COUNTER PLUS #NAME #NUM\r\n
COUNTER PLUS test 1\r\n
```
##### *Return:*
```
TOGO_S1TOGO_E\r\n
```

###2. 计数器减去某个数字，默认-1
```
COUNTER MINUS #NAME #NUM\r\n
COUNTER MINUS test 1\r\n
```
##### *Return:*
```
TOGO_S1TOGO_E\r\n
```

###3. 计数器获取一个值
```
COUNTER GET #NAME\r\n
COUNTER GET test\r\n
```
##### *Return:*
```
TOGO_S1TOGO_E\r\n
```

###4. 计数器初始化
```
COUNTER RESET #NAME\r\n
COUNTER RESET test\r\n
```
##### *Return:*
```
TOGO_S1TOGO_E\r\n
```

##队列模块协议
###1. 从左边插入一个记录
```
QUEUE LPUSH #NAME #VALUE #PRIORITY(1,2,3 优先级)\r\n
QUEUE LPUSH test 1234\r\n
QUEUE LPUSH test 1234 2\r\n
```

##### *Return:*
```
TOGO_STOGO_OKTOGO_E\r\n  #成功
TOGO_STOGO_FAILTOGO_E\r\n  #失败
```

###2. 从右边插入一个记录
```
QUEUE RPUSH #NAME #VALUE #PRIORITY(1,2,3 优先级)\r\n
QUEUE RPUSH test 1234\r\n
QUEUE RPUSH test 1234 2\r\n
```
##### *Return:*
```
TOGO_STOGO_OKTOGO_E\r\n  #成功
TOGO_STOGO_FAILTOGO_E\r\n  #失败
```
###3. 从左边获取一个记录
```
QUEUE LPOP #NAME\r\n
QUEUE LPOP test\r\n
```
##### *Return:*
```
TOGO_S123TOGO_E\r\n  #成功
TOGO_STOGO_NULLTOGO_E\r\n  #空
TOGO_STOGO_FAILTOGO_E\r\n  #失败
```
###4. 从右边获取一个记录
```
QUEUE RPOP #NAME\r\n 
QUEUE RPOP test\r\n
```
##### *Return:*
```
TOGO_S123TOGO_E\r\n  #成功
TOGO_STOGO_NULLTOGO_E\r\n  #空
TOGO_STOGO_FAILTOGO_E\r\n  #失败
```

###5. 获取一个队列的总记录数
```
QUEUE COUNT #NAME\r\n 
QUEUE COUNT test\r\n
```
##### *Return:*
```
TOGO_S18TOGO_E\r\n  #成功
TOGO_STOGO_FAILTOGO_E\r\n  #失败
```
###6. 获取一个队列的状态
```
QUEUE STATUS #NAME\r\n
QUEUE STATUS test\r\n
```
##### *Return:*
```
TOGO_Stotal_elt:18;total_block:1;total_hit:22;total_write:20;
total_read:2;total_size:2097200TOGO_E\r\n  #成功
TOGO_STOGO_FAILTOGO_E\r\n  #失败
```

##内存锁模块协议：
###1. LOCK操作
```
LOCK LOCK #NAME\r\n
LOCK LOCK test\r\n
```
##### *Return:*
```
TOGO_S1TOGO_E\r\n  #成功
TOGO_STOGO_FAILTOGO_E\r\n  #失败
```

###2. UNLOCK操作
```
LOCK UNLOCK #NAME\r\n
LOCK UNLOCK test\r\n
```
##### *Return:*
```
TOGO_S0TOGO_E\r\n  #成功
TOGO_STOGO_FAILTOGO_E\r\n  #失败
```

###3. 获取一把锁的状态
```
LOCK STATUS #NAME\r\n
```
##### *Return:*
```
TOGO_Sstatus:1;total:3;total_lock:3;total_unlock:0TOGO_E\r\n  #成功
TOGO_STOGO_FAILTOGO_E\r\n  #失败
```

##缓存模块协议：
###1. 缓存set操作(如果元素存在，则替换)
```
CACHE SET #key #expires(缓存时间 0为永久，秒为单位) #vlen(例如5，则value的长度为5)\r\n
abced  #value值长度为5
```

##### *Return:*
```
TOGO_SOKTOGO_E\r\n  #成功
TOGO_STOGO_FAILTOGO_E\r\n  #失败
```

###2. 缓存add操作（如果元素存在，则不替换）
```
CACHE ADD #key #expires(缓存时间 0为永久，秒为单位) #vlen(例如5，则value的长度为5)\r\n
abced  #value值长度为5
```

##### *Return:*
```
TOGO_SOKTOGO_E\r\n  #成功
TOGO_STOGO_FAILTOGO_E\r\n  #失败
TOGO_STOGO_IS_EXISTTOGO_E\r\n  #元素已经存在
```

###3. 缓存replace操作（如果元素存在，则替换）
```
CACHE REPLACE #key #expires(缓存时间 0为永久，秒为单位) #vlen(例如5，则value的长度为5)\r\n
abced  #value值长度为5
```

##### *Return:*
```
TOGO_SOKTOGO_E\r\n  #成功
TOGO_STOGO_FAILTOGO_E\r\n  #失败
TOGO_STOGO_NOT_EXISTTOGO_E\r\n  #元素不存在
```

###4. 缓存get操作
```
CACHE GET #key\r\n
```

##### *Return:*
```
TOGO_S10(长度)TOGO_E/r/nAAAAAAAAAA(值)  #成功
TOGO_STOGO_FAILTOGO_E\r\n  #失败
TOGO_STOGO_NOT_EXISTTOGO_E\r\n  #元素不存在
```


###5. 缓存flush操作
```
CACHE FLUSH\r\n
```

##### *Return:*
```
TOGO_STOGO_OKTOGO_E\r\n  #成功
TOGO_STOGO_FAILTOGO_E\r\n  #失败
```
##返回值协议
####1. 操作成功
```
TOGO_STOGO_OKTOGO_E\r\n
TOGO_S#value#TOGO_E\r\n
```
####2. 操作失败<br>
```
TOGO_STOGO_FAILTOGO_E\r\n
```
####3. 返回为空<br>
```
TOGO_STOGO_NULLTOGO_E\r\n
```
####4. 命令行太长<br>
```
TOGO_STOGO_COMMAND_TOO_BIGTOGO_E\r\n
```
####5. 需要发送/接收的内容太大<br>
```
TOGO_STOGO_TOO_BIGTOGO_E\r\n
```
####6. 元素已经存在<br>
```
TOGO_STOGO_IS_EXISTTOGO_E\r\n
```

####7. 元素不存在<br>
```
TOGO_STOGO_NOT_EXISTTOGO_E\r\n
```
