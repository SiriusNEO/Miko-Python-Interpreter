# Toyosatomimi no Miko

关于这台Python解释器，可以看一看下面的内容

~~（标题是什么呢没错就是它的名字）~~

### Part 1 代码结构

---

##### `Bigint`

封装好的高精度大整数类

- 四则运算
- 基于 `string`、`int` 类型的构造以及转换



##### `Base`

对应 Python 中的 Object 概念，整合了所有基本数据类型

```C++
_null _bool _int _float _str _list _none
```

其中 `_null` 表示这是一个未赋值的左值变量，如果将其用于运算将因为 Undefined 报错

- 基于相应类型的构造

- 不同类型间的转换

- 各种奇怪的运算符

- 一些数据变量：

  （本来想用 union 但是这玩意儿会隐式删除我的拷贝构造函数，于是作罢）

  ```C++
  baseType; //类型
  boolData intData floatData strData //基本数据类型
  std::vector<Base*> listData; //用shared_ptr大概能防止内存泄漏
  nameData; //变量名，当且仅当其为左值才拥有
  ```

- 一些表示其属性的变量：

  ```C++
  bool isLeftValue; //是否是个左值
  bool isImmutable; //是否为可变对象，如果尝试对不可变对象进行内容修改会报错
  ```

- 其它的一些工具函数，方便代码实现

  

##### `TrueAdministrator`    
全局管理类，用于管理变量内存的分配、变量与函数的记录、flowStatement的管理等

（总之所有“全局”概念的东西都丢到这里了）

（注：其成员对用户均可见）

- 索引：

  变量索引，分为 global、temporary、LC

  函数索引，储存了其参数表以及suite结点，调用时通过名字找到

- 环境变量：（表示当前全局状态）

  ```C++
  
  int depth; //当前递归深度
  std::unordered_map <Python3Parser::ParametersContext*, std::vector<Base> > defaultArguments; //储存所有函数的默认变量
  
  //extra
  bool passByRefernce; //用于list中的引用概念，信号指示器
  					 //较naive的实现，因此请不要在下标中调用复杂函数以免信号冲突
  int LCdepth;         //LC递归深度
  ```

- 暂存器：（用于全局间的信息传递）

  ```C++
  std::vector<flowStmtType> flowStmtStack; //暂存当前的flow命令，用vector写起来比较优美（虽然似乎三个bool变量一样能达到）
  std::vector<std::string> kwTable; //暂存用户调用函数时的所有keyword argument
  std::vector<Base> returnTmp; //暂存返回值
  
  //extra
  Python3Parser::TestContext* LCoutExpr[maxDepth]; //暂存LC输出表达式
  std::vector<Python3Parser::Lc_exprContext*> LCStack[maxDepth]; //暂存LC
  std::vector<Base> LCreturnTmp[maxDepth]; //暂存LC返回值
  std::vector<int> trace; //追踪多维list所用
  						//较naive的实现，因此请不要在下标中调用复杂函数以免信号冲突
  ```

- 两个变量操作的成员函数：

  ```C++
  newVariable(); 
  //新建变量，若在列表推导式中则在LC中新建，若当前在函数体内则在temporary中新建，否则在global新建
  //if while for所开变量均为局部变量！
  getVariable();
  //读变量，查找顺序与新开变量相同
  ```

  

### Part 2 附加功能

---

以下所有**标明不支持的语法**将报 [Sorry] 的错误，

~~这是作者告诉你：不是你菜，而是我自己太菜~~



##### 较简单的报错系统

该系统与 Python3 中的错误归类并不一致

由于不太会改 Lexer ，失配的语句不会提示语法错误，而是忽略并继续正常执行

由于报错系统细节太多，所以这部分还比较 naive

发生错误会输出错误提示信息，并直接 **CE**

错误类型大致归类如下：

```C++
[Sorry] //这严格上说并不算一个Error，只是写解释器的人太菜了所以不支持此语法
[UndefinedError] //出现了未定义的对象，注意Python变量需要先通过赋值方式定义，函数需要通过def开头的表达式定义
[ArgumentError] //参数调用错误，包括参数数目错误、不合法的参数类型、无法找到的keyword argument
[OperatorError] //运算符错误，很可能是这些类型不支持这种运算
[ZroDivisionError] //除零错
[AssignmentError] //赋值过程中发生的错误
[TypeError] //类型与其行为的错误
[IndexError] //下标越界
[SyntaxError] //语法错误，主要针对一些小细节部分的不合法
```



##### tuple 与 list

两种 Python 基本的数据类型，支持：

- 利用圆括号、方括号快速构建
- 运算符：+ 与 *
- 下标访问，多维数组
- 与之前类型间的转换（bool和str）
- 一些新的内置函数：tuple、list、type、range、len

**注意**以下事项：

- tuple、list 之间的赋值与 Python3 保持一致：即为引用传递，指向同一区域
- **与 Python3 不同的是**，这里声明一个 tuple **必须带圆括号**，这是为了尊重之前的 testlist
- **与 AC 代码不同的是**，函数多返回值会返回一个 tuple
- **与 Python3 不同的是**，这里的 range 返回一个 tuple，这是因为懒惰的作者懒得再写更多可迭代类型了（误）
- 初始化单个元素的 tuple  必须加逗号，这点与 Python3 保持一致
- **不支持** list 转换为 str，~~因为作者看了这个语法后感觉很蠢没啥用~~

- 最好不要在下标中调用复杂的函数（尤其是**该函数中有可能再次进行下标访问**）



##### 切片

实现了 list 后就自然想搞这个了

不过也比较 naive，因为架构上的劣根性，具体看下面吧：

- 切片访问操作，语法与 Python3 相同： `start_index:stop_index:step`
- 连续切片，规则与 Python3 相同
- 切片也可以 +=、*=，规则与 Python3 相同
- 切片赋值，规则与 Python3 相同，**但注意的是**，目前**只支持步长为+1**的切片赋值，这是因为本身内部实现以及整个 g4 架构上的问题



##### for 循环

while 配合 list 用起来并不是很很方便，于是搓了个这个

基本与 Python3 相同，具体注意下面两条：

- 循环变量是全局的，与 Python3 相同

- 注意**最好不要**在循环语句内修改你正在迭代的对象，这个解释器的逻辑为：拷贝一份迭代对象进行迭代，也就是说你对其的修改并不会影响循环，这点与 Python3 不同（我也不知道 Python 里这个问题的处理是什么逻辑，总之这在 Python 里也是一种危险行为）

  

  

##### 列表推导式

List Comprehension，简称 LC，用于快速生成有规律的 list：

- 生成一个 list 对象
- 语法与 Python3 大致相同，也支持后置 if 与嵌套、多个连一起
- LC 中的循环变量在其结束后将会被释放，并且在未结束时将一直能被访问，这意味着嵌套 LC 时**最好不要**使用同名变量，同时**最好不要**在 if 中套非常复杂的语法



### Part 3 其它

- 真不错，这个大作业真不错，因垂丝汀（写上头了）
- 下发数据有一点点弱，建议下次给些强的，不然对着 OJ 的运行结果真的不好调 TAT
- 建议别卡常（
- 感谢两位助教以及一些同学给予我很大的帮助