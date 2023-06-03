军棋人工智能qq交流群:806149422 <br>

engine1为3.0代码，目前进度90%，和2.3测了10盘，5胜3负2和 <br>
本来打算出一个95%的版本，和90%测试8胜1负1和，但是和2.3测试连输5把，气的我把写了半年多的代码全部删掉重写

感觉这个项目是突变型的而不是渐进型的，要么一直徘徊在弱智的水平，一旦突破了某个点棋力就会迅速提升到一个比较高的水平。我想到了一个全新的算法，就看能不能突变了。

关于下面的编译说明，我没有做什么一步一步的教程，对于linux编译环境和Makefile非常熟悉的应该稍微看一下就能编译出来了，新手就不必尝试了，有些东西不熟悉的话容易浪费时间。

重写的代码还是不行，没有本质的提升，做不出来。


# 四国军棋
基于GTK+ 3界面库，使用eclipse cdt开发环境，win10
windows下需要安装64位的msys2

软件分为界面和引擎2部分，需要分别编译分别启动
      
按照当前思路开发下去工作量太大，先暂停一段时间。    

![demo](https://github.com/pfysw/JunQi/raw/master/GUI/res/demo.png)
## 编译
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