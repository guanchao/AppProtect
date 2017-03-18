# AppProtect
整理一些app常见的加固方法，包括java层、native层和资源文件加固等

1.HiddenMethod
隐藏dex文件的method

2.DexEncrypt
对dex文件整体加密解密

3.SectionEncrypt
对so文件的指定section进行整体加密解密

4.MethodEncrypt
对so文件的指定方法进行加密解密

5.Dalvik运行时篡改字节码 
在运行时动态修改dex中方法的字节码

6.通过hook重定向native方法
使用cydia substrate hook框架重定向native方法，达到保护native方法的目的

7.动态注册native方法
自己注册native方法，隐藏带有Java_xxx_xxx_xxx的函数名特征