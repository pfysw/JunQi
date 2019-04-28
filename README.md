# 四国军棋
基于GTK+ 3界面库，使用eclipse cdt开发环境，
windows下需要安装msys2

软件分为界面和引擎2部分，需要分别编译分别启动
      
按照当前思路开发下去工作量太大，先暂停一段时间。    

![demo](https://github.com/pfysw/JunQi/raw/master/GUI/res/demo.png)
## 编译
如使用的是32位msys2，直接在eclipse导入工程即可

需要设置GTK+的环境变量地址为F:\msys32\mingw32\bin;  （必须放在开头）

需要设置gcc的环境变量地址为F:\msys32\usr\bin;

新建C语言工程时编译器选择Cygwin GCC (根据你在msys2上安装的编译器)

msys2环境搭建可以参考下文

https://blog.csdn.net/qiuzhiqian1990/article/details/56671839

如编译出现错误，请参照下文的GTK+环境搭建进行修改

https://blog.csdn.net/pfysw/article/details/81048379

另外代码的相关说明请参考我写的博客

https://blog.csdn.net/pfysw/article/category/7890775    

## 测试战绩
目前稍微有点会下棋了，但是仍然处在弱智阶段，大局观太差
![战绩](https://github.com/pfysw/JunQi/raw/master/GUI/res/result.png)