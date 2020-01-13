# MatchaOS
阅读《一个64位操作系统的设计与实现》, 自制操作系统. 

## 环境
### 编译环境
操作系统: CentOS

Mac或Windows操作系统推荐在Docker中安装CentOS镜像, 在容器中编译.

示例:

创建并保存镜像
```bash
docker pull centos:centos8
docker run -it -v MatchaOS_path:/root/MatchaOS -h master centos:centos8 /bin/bash

yum install -y gcc
yum install -y make

# nasm 可能需要配置yum源, 参考https://www.nasm.us/
yum install -y nasm

# 保存当前镜像
docker ps -l    // 获取当前镜像ID
docker commit image-id centos:matchaos
```
编译
```bash
# 以特权模式启动镜像
docker run -it --privileged=true -v MatchaOS_path:/root/MatchaOS -h master centos:matchaos /bin/bash
cd /root/MatchaOS/src/ && make
```

### 运行
编译后, 可以直接执行`start.sh`脚本, 在boches虚拟机启动matchaOS.
```bash
cd MatchaOS_path
sh start.sh
```

> Mac中, 如果使用Qemu的话, 启动会出现问题, 暂时没有解决.

## 参考资料
1. [(参考书源代码) MINE操作系统](https://gitee.com/MINEOS_admin)
2. [清华大学操作系统实验ucore_os_lab](https://github.com/chyyuu/ucore_os_lab)
