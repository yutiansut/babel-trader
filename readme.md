# Babel Trader

## 简介
传统金融的二级市场交易中，由于历史原因以及一些超低延时交易的需求，大量使用了sdk的形式提供API(C++/JAVA的库)。对有能力的开发者，对接此类API不是太大的问题，但是对于大多数缺乏开发能力的使用者来说，对接这类API，是十分痛苦的事情。

数字货币交易所方面，大都提供了ws/restful形式的接口，方便了大多数的交易员用自己喜欢的语言对接，但是各个交易所的接口，交易接口字段并不统一，需要花费时间，去逐个对接不同的交易所。

Babel Trader 将一些常用的交易API统一变为ws/restful服务的形式，并提供统一的行情和交易接口字段。

## 传统单进程交易框架
过去，为了统一不同的API，常用做法是，在同一个进程中，将API封装，而策略逻辑，通常也是运行在同一个进程当中：
```
====================
      API1 ------------> API1 server
      API2 ------------> API2 server
              ......
      APIn ------------> APIn server
     Strategy
    Backtesting
====================
```
此种做法，提供了封装好的统一API，但是缺点也是显而易见的：
1. 当有某个API需要更新时，不得不停掉整个系统
1. 单例程序，容灾性低下，一个bug，可能导致整个系统dump掉
1. 扩展性较差

## Babel Trader的做法
推荐架构：
```
client1 ------> |================================| ---> BabelTrader service 1 ---> API1 server
client2 ------> |         系统其他service         | ---> BabelTrader service 2 ---> API2 server
        ......  |  网关, 风控，分账户,行情落库...  |    ......
clientn ------> |================================| ---> BabelTrader service n ---> APIn server
```
使用Babel Trader的服务，提供了统一的 ws/restful 接口，方便开发者实现代码解耦和自己的分布式系统。  
缺点:   
由于通用性考虑, 使用了ws传输json格式数据, 会比私有协议存在更大的延时


## 使用
对于想要直接使用的用户, 可以从 github 的 release中, 下载已经编译好的二进制文件, 更改一下config里的账户和密码配置, 便可直接运行。  

win64:  
1. 解压下载的release包, 解压其中的 babeltrader-*-win64.zip  
1. 更改config目录里, 对应市场的账户和密码, 以及订阅配置  
1. 运行babeltrader开头的文件, 便得到了对应市场的上手服务  

linux:  
1. 解压下载的release包, 解压其中的 babeltrader-*-linux.zip  
1. 更改config目录里, 对应市场的账户和密码, 以及订阅配置
1. 进入解压后的babeltrader-*-linux目录层中, export LD_LIBRARY_PATH=./
1. 运行babeltrader开头的文件, 便得到了对应市场的上手服务


## 文档和demo
文档: doc目录  
demo: demo目录中, 有不同语言的例子: python, java, golang 等  

## Build
babel trader使用了c++和golang, 当前ctp, xtp上手服务使用c++编写, okex上手服务使用golang编写  

c++部分整个工程，直接使用CMake构建即可，注意先安装依赖性以及初始化第三方库。对于不熟悉cmake的朋友，可以直接按照下面步骤使用即可。  

#### Linux
安装依赖项：sudo apt-get install libssl-dev  
首次构建前: 进入根目录，运行 git submodule update --init  
构建: 进入工程的根目录，运行 build.sh  
编译: 进入build目录, 运行 make 即可  

#### Windows
先安装好 vcpkg (https://github.com/Microsoft/vcpkg.git), 并在环境变量中, 设置VCPKG_ROOT为vcpkg所在目录。  

安装依赖项：vcpkg.exe install openssl:x64-windows zlib:x64-windows  
首次构建前: 进入根目录, 运行 git submodule update --init  
构建: 进入工程的根目录, 运行build.bat  

注意:   
如果是自己构建, 需要将目录中的 config-template 改名为 config, 并正确配置其中的账号。make时, 会自动将config拷贝到build/bin目录中。 具体的配置文件字段，参考doc文档。  

golang部分  
1. go get github.com/MuggleWei/babel-trader  
1. 进入src/babeltrader-okex-v3-*, go build  

