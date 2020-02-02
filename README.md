# MatchaOS
阅读《一个64位操作系统的设计与实现》, 自制操作系统. 

## 环境
### 编译环境
操作系统: CentOS

Mac或Windows操作系统推荐在Docker中安装CentOS镜像, 在容器中编译.

示例:

```bash
# 构建镜像
docker build -t centos:matcha .

# 启动容器
docker run -id -h master --privileged --name matcha -v MatchaOS_Path:/home/matcha centos:matcha

# 编译
docker exec -it matcha bin/bash
$ make clean & make
```

### 运行
编译后, 运行`start.sh`脚本, 在Qemu虚拟机中启动matchaOS.
```bash
sh start.sh
```

> 和Bochs相比, Qemu可以模拟更多的CPU和硬件设备, 并有着更快的运行速度, 尤其是在多处理器(MP)模式下. 并且Qemu可以结合GDB, 对C代码进行调试
> (虽然Bochs也支持但是比较麻烦)

## 参考资料
1. [(参考书源代码) MINE操作系统](https://gitee.com/MINEOS_admin)
2. [清华大学操作系统实验ucore_os_lab](https://github.com/chyyuu/ucore_os_lab)
