# php进程内存储扩展

## 特性

* 进程内存储方式，不支持夸进程通信
* 永驻内存的，不是单请求生命周期
* 超高性能define函数，进城周期内复用

## inproc_set

```
boolean inproc_set(string $key, mixed $value [, int $expire = 0])
```

### 参数说明

* key ```存储使用的key值 string类型```
* value ```存储的内容```
* expire ```过期时间 默认是永久保存```

### 返回值

```
bool类型 成功返回true 失败返回false
```

### 实例

* 不设值过期时间
```
var_dump(inproc_set('inproc_set', 'inproc_set'));
var_dump(inproc_get('inproc_set'));
//输出
bool(true)
string(16) "inproc_set"
```

* 设置过期时间
```
var_dump(inproc_set('inproc_set', 'inproc_set', 5));
var_dump(inproc_get('inproc_set'));
sleep(6);
var_dump(inproc_get('inproc_set'));
//输出
bool(true)
string(16) "inproc_set"
bool(false)
```

## inproc_inc

```
boolean inproc_inc(string $key, int $step)
```

### 参数说明

* key ```存储使用的key值 string类型```
* step ```每次的增加值 int类型```

## 返回值

```
bool类型 成功返回true 失败返回false
```

### 实例

```
var_dump(inproc_inc('inproc_inc', 10));
var_dump(inproc_inc('inproc_inc', 10));
var_dump(inproc_inc('inproc_inc', 1));
var_dump(inproc_get('inproc_inc'));
```

#### 输出

```
bool(true)
bool(true) 
bool(true) 
int(21)
```


## inproc_get

```
mixed inproc_get(string $key)
```

#### 参数说明

* key ```存储使用的key值 string类型```

### 返回值

```
inproc_inc 存入的值会返回 int 类型
没有查询到key值会返回 false
```

#### 实例

```
//inproc_inc
inproc_inc('inproc_inc', 10);
var_dump(inproc_get('inproc_inc'));
//inproc_set
inproc_set('inproc_set', 'inproc_set', 5);
var_dump(inproc_get('inproc_set'));
//no key
var_dump(inproc_get('inproc_get'));
```

#### 输出

```
int(10) 
string(16) "inproc_set" 
bool(false)
```

## inproc_del

```
boolean inproc_del(string $key)
```

#### 参数说明

* key ```存储使用的key值 string类型```

#### 返回值

```
删除成功返回 true 
删除失败返回 false
```

#### 实例

```
inproc_inc('inproc_inc', 10);
var_dump(inproc_del('inproc_inc'));
```

#### 输出

```
bool(true)
```

## inproc_exists

```
boolean inproc_exists(string $key)
```

#### 参数说明

* key ```存储使用的key值 string类型```

#### 返回值

```
存在返回 true 
不存在返回 false
```

#### 实例

```
inproc_set('inproc_set', 10);
var_dump(inproc_exists('inproc_set'));
```

#### 输出

```
bool(true)
```



## inproc_define

```
boolean inproc_define(string $key, mixed $value)
```

__注意，inproc_define定义常量后，如果要更新，必须重启php-fpm才能生效！！！__ ```务必注意！！！```

### 参数说明

* key ```存储使用的key值 string类型```
* value ```存储的内容```


### 返回值

```
bool类型 成功返回true 失败返回false
```

### 实例

```
$key1 = __FILE__."1";

if(!defined($key1)){
        inproc_define($key1, true);

        for($i=0; $i<1000; $i++){
                $key = "key$i";
                inproc_define($key, "val$i");
                //类似的define($key, "val$i")
        }
}


var_dump(key100);


输出：
string(6) "val100"


```


## 与apcu的简单性能对比

```
//inproc
//插入100w条数据=0.21569800376892
//查找100w条数据=0.14358401298523

//apcu
//插入100w条数据=0.58673405647278
//查找100w条数据=0.29949307441711
```


## 高性能define和普通define简单性能对比

```
///////a.php

$key1 = __FILE__."1";

if(!defined($key1)){
        inproc_define($key1, true);

        for($i=0; $i<1000; $i++){
                $key = "key$i";
                inproc_define($key, "val$i");
        }
}

///////b.php

$key1 = __FILE__."1";

if(!defined($key1)){
        define($key1, true);

        for($i=0; $i<1000; $i++){
                $key = "key$i";
                define($key, "val$i");
        }
}


//inproc_define
ab -n1000 -c2 -k "http://192.168.98.131/a2.php"

Total transferred:      181000 bytes
HTML transferred:       19000 bytes
Requests per second:    2120.27 [#/sec] (mean)
Time per request:       0.943 [ms] (mean)
Time per request:       0.472 [ms] (mean, across all concurrent requests)
Transfer rate:          374.78 [Kbytes/sec] received



//php define
Total transferred:      181000 bytes
HTML transferred:       19000 bytes
Requests per second:    550.68 [#/sec] (mean)
Time per request:       3.632 [ms] (mean)
Time per request:       1.816 [ms] (mean, across all concurrent requests)
Transfer rate:          97.34 [Kbytes/sec] received

```


