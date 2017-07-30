# NChat #
按照 `Qt及Qucik开发实战精解`一书写的简易局域网聊天程序
# Review #

----------



- **Qt tr 函数的作用**
![](http://i.imgur.com/seWXmDy.png)
[http://blog.csdn.net/mfc11/article/details/6591134](http://blog.csdn.net/mfc11/article/details/6591134 "转自")

----------
# 有关const成员、static成员、const static成员的初始化：(这都能忘) #


1. const成员：只能在构造函数后的初始化列表中初始化



2. static成员：初始化在类外，且不加static修饰



3. const static成员：类只有唯一一份拷贝，且数值不能改变，初始化在类外,要加const修饰