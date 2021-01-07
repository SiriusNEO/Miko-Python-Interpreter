# About

Miko的简要说明文档

### Part 1 代码结构

`Bigint`：封装好的高精度大整数类

- 四则运算
- 基于 `string`、`int` 类型的构造以及转换



`Base`：对应Python中的Object概念，整合了所有基本数据类型

```C++
_null _bool _int _float _str _list _none
```

其中 `_null` 表示这是一个未赋值的左值变量，如果将其用于运算将因为 Undefined 报错

- 基于相应类型的构造
- 不同类型间的转换
- 各种奇怪的运算符
- 一些表示其属性的变量
- 本来想用union但是这玩意儿会隐式删除我的拷贝构造函数，于是作罢

`TrueAdministrator`    全局管理类，用于管理变量内存的分配、变量与函数的记录、语句的

