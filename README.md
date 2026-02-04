# _fileserver  : 程序源码;
# _tcpputfiles : 程序源码;
# my_code/Executable_file:可执行文件夹

# 环境配置:
1) 需要安装g++ 编译器;     
sudo dnf install -y gcc-c++

2) 开通5005端口;
开通5005端口
firewall-cmd --zone=public --add-port=5005/tcp --permanent
移去5005端口
firewall-cmd --zone=public --remove-port=5005/tcp --permanent


# _fileserver:    program source code;
# _tcpputfiles:   program source code;
# my_code/Executable_file: executable folder

# Environment Configuration:
1) Need to install GCC compiler;
sudo dnf install -y gcc-c

2) Open port 5005;
Open port 5005
firewall-cmd --zone=public --add-port=5005/tcp --permanent
Remove port 5005
firewall-cmd --zone=public --remove-port=5005/tcp --permanent






