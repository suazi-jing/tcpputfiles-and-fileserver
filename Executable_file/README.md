# fileserver :编译脚本为:./fileserver /log/idc/fileserver.log 5005 

# <参数解释>
./fileserver:可执行文件
/log/idc/fileserver.log:日志文件 
5005:通信端口;

# fileserver: Compile script as: ./fileserver /log/idc/fileserver.log 5005

# <Parameter Explanation>
./fileserver: executable file
/log/idc/fileserver.log: log file
5005: communication port;

# tcpputfiles :编译脚本为:./tcpputfiles /log/idc/tcpputfiles_surfdata.log "<ip>192.168.218.129</ip><port>5005</port><clientpath>/tmp/client</clientpath><ptype>1</ptype><srvpath>/tmp/server</srvpath><andchild>true</andchild><matchname>*.xml,*.txt,*.csv,*.json</matchname><timetvl>10</timetvl><timeout>50</timeout><pname>tcpputfiles_surfdata</pname>"

# <帮助文档>

Using:/project/tools/bin/tcpputfiles logfilename xmlbuffer


logfilename:   本程序运行的日志文件。
xmlbuffer:     本程序运行的参数，如下：
ip:            服务端的IP地址。
port:          服务端的端口。
ptype:         文件上传成功后的处理方式：1-删除文件；2-移动到备份目录。
clientpath:    本地文件存放的根目录。
clientpathbak: 文件成功上传后，本地文件备份的根目录，当ptype==2时有效。
andchild:      是否上传clientpath目录下各级子目录的文件，true-是；false-否，缺省为false。
matchname:     待上传文件名的匹配规则，如"*.TXT,*.XML"
srvpath:       服务端文件存放的根目录。
timetvl:       扫描本地目录文件的时间间隔，单位：秒，取值在1-30之间。
timeout:       本程序的超时时间，单位：秒，视文件大小和网络带宽而定，建议设置50以上。
pname:         进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。


# tcpputfiles: compile script as: ./tcpputfiles /log/idc/tcpputfiles_surfdata.log "<ip>192.168.218.129</ip><port>5005</port><clientpath>/tmp/client</clientpath><ptype>1</ptype><srvpath>/tmp/server</srvpath><andchild>true</andchild><matchname>*.xml,*.txt,*.csv,*.json</matchname><timetvl>10</timetvl><timeout>50</timeout><pname>tcpputfiles_ surfdata</pname>"

# <Parameter Explanation>

Using:/project/tools/bin/tcpputfiles logfilename xmlbuffer

logfilename:    The log file for this program.
xmlbuffer:      The parameters for this program, as follows:
ip:             The server's IP address.
port:           The server's port.
ptype:          The way to handle files after successful upload: 1 - Delete the file; 2 - Move to backup directory.
clientpath:     The root directory where local files are stored.
clientpathbak:  The root directory for local file backups after successful upload, effective when ptype==2.
andchild:       Whether to upload files from all subdirectories under clientpath, true - yes; false - no, default is false.
matchname:      The matching rule for file names to be uploaded, e.g., "*.TXT,*.XML"
srvpath:        The root directory where server files are stored.
timetvl:        The time interval for scanning local directory files, in seconds, ranging from 1 to 30.
timeout:        The timeout for this program, in seconds, depending on file size and network bandwidth, recommended to set above 50.
pname:          The process name, preferably easy to understand and different from other processes to facilitate troubleshooting.
