# NChat #
按照 `Qt及Qucik开发实战精解`一书写的简易局域网聊天程序
# Review #

----------
- **程序使用的主要机制**


与之前自己写的TCP程序不同，这是一个P2P程序，自身既作为服务端又作为客户端。

----------

- **主要功能的实现方法**

文件传输：WIDGET类上点击传输文件按钮，实例化一个SERVER。使用槽和信号机制将选择的文件名返回给WIDGET，此时实例化一个CLIENT。同时WIDGET发送包含文件名信息的广播，CLIENT接收到广播后尝试连接之前的SERVER，建立TCP连接并传输文件

----------

- **QT发布程序**

之前写的程序都只上了源码，这次找了好久才找到一个比较方便发布程序的办法：使用QT自带的windeployqt程序。

在cmd窗口里cd到可执行程序所在目录，执行 
> windeploy [程序].exe

此工具会自动将所需要的dll引入（终于不用自己一个一个加dll了）

但是（可能）由于我电脑环境的原因，exe文件还是运行不起来，等解决了再更新吧

----------

- **QT中获得当前用户名及主机名**

用户名：qgetenv("USER") 或者 qgetenv("USERNAME") ，取决于自己电脑上的环境变量

主机名：QHostInfo::localHostName();

----------

- **Qt tr 函数的作用**
![](http://i.imgur.com/seWXmDy.png)
[http://blog.csdn.net/mfc11/article/details/6591134](http://blog.csdn.net/mfc11/article/details/6591134 "转自")

----------
# 有关const成员、static成员、const static成员的初始化：(这都能忘) #


1. const成员：只能在构造函数后的初始化列表中初始化



2. static成员：初始化在类外，且不加static修饰



3. const static成员：类只有唯一一份拷贝，且数值不能改变，初始化在类外,要加const修饰