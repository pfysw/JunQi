engine1为3.0代码，已不再开发，和老代码测试没有本质的提升，感觉很难跳出50%胜率。

此代码已经不再维护，大家不要再考虑在我的代码基础上做，容易浪费时间。

# 四国军棋
基于GTK+ 3界面库，使用eclipse cdt开发环境，win10
windows下需要安装64位的msys2

软件分为界面和引擎2部分，需要分别编译分别启动
      
按照当前思路开发下去工作量太大，先暂停一段时间。    

![demo](https://github.com/pfysw/JunQi/raw/master/GUI/res/demo.png)
## 编译
关于下面的编译说明，我没有做什么一步一步的教程，对于linux编译环境和Makefile非常熟悉的应该稍微看一下就能编译出来了，新手就不必尝试了，有些东西不熟悉的话容易浪费时间。

使用的是64位msys2，直接在eclipse导入工程即可，需在工程目录下新建一个bld文件夹

GUI已经支持makefile编译，ENGINE工程比较简单没什么依赖还是由eclipse自动编译

需要设置GTK+的环境变量地址为D:\msys64\mingw64\bin （必须放在开头）

需要设置gcc的环境变量地址为D:\msys64\usr\bin

新建C语言工程时编译器选择Cygwin GCC (根据你在msys2上安装的编译器)

msys2环境搭建可以参考下文

https://blog.csdn.net/qiuzhiqian1990/article/details/56671839

如编译出现错误，请参照下文的GTK+环境搭建进行修改（有了makefile后，这个已经失效，谨慎参考）

https://blog.csdn.net/pfysw/article/details/81048379

另外代码的相关说明请参考我写的博客

https://blog.csdn.net/pfysw/article/category/7890775    

## 测试战绩
目前稍微有点会下棋了，但是仍然处在弱智阶段，大局观太差
![战绩](https://github.com/pfysw/JunQi/raw/master/GUI/res/result.png)