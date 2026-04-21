python接口使用说明

KW-MINI系列相机调用API时，支持C/C++/python/C#接口，现以python为例。

使用步骤：
1、找到想使用的python版本对应的Release,如您电脑使用python3.10，即可将本文件夹下的Release_python3.10里的所有文件同时放入python解释器的Lib/site-packages文件夹下。
2、打开一款编译器如：PyCharm，打开本文件夹下的调用示例文件：example_python.py,并使用python3.10解析器,运行即可。

注意：接口模块名为：camera
例如：
import camera
#连接相机
camera.DfConnect(ip)

详细使用见example_python示例代码。






















