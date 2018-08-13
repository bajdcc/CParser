# CParser

**本项目是[CMiniLang](https://github.com/bajdcc/CMiniLang)**的VS编译版本。

自己做的toys，纯属娱乐 :)

本项目中的Lexer由我自己编写，参考了[CEval](https://github.com/bajdcc/CEval)中的部分代码。Parser和VM暂时是使用**write-a-C-interpreter**项目中的代码，自举文件**xc.txt**也是。

后期：Parser自己实现生成AST，VM改善指令与VMM兼容，xc.txt中尽量实现AST。

## 主要功能

1. 解析C文件
2. 生成语法树
3. 构造指令集
4. 建立虚拟机

## 进度

1. Lexer（LL手写识别，比regex库高效）
   1. 识别数字（科学计数+十六进制）
   2. 识别变量名
   3. 识别空白字符
   4. 识别字符（支持所有转义）
   5. 识别字符串（支持所有转义）
   6. 识别注释
   7. 识别关键字
   8. 识别操作符
   9. 错误处理（快速失败）
2. Parser
   1. 识别函数
   2. 识别枚举
   3. 识别表达式
   4. 识别基本结构
3. 虚拟机
   1. 实现虚页（已实现，分代码段，数据段，栈，堆）
   2. 实现MALLOC（已实现，参考[CLib::memory.h](https://github.com/bajdcc/learnstl/blob/master/code/02/memory.h)）

## 截图

### 词法分析

![](https://pic4.zhimg.com/v2-12fcbe73a8340d20a9488ae0228ff11f.png)

### 解释器

![](https://pic1.zhimg.com/v2-855b2e604a19e44a9f0f52e2a0eca010_r.png)

### 运行

![](https://raw.githubusercontent.com/bajdcc/CParser/master/screenshots/1.gif)

## 参考

1. [write-a-C-interpreter](https://github.com/lotabout/write-a-C-interpreter)
