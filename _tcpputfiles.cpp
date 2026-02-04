/*
    tcppuutfiles.cpp 采用tcp文件上传客户端 采用io复用的方式;
    该程序用于学习分享,不承担任何法律风险;
    程序运行在Linux上, tcppuutfiles.cpp 是程序源码,不包含开发框架_public.h;
    作者会附带编译后可执行文件 tcppuutfiles
    作者:赵梓涵

    tcppuutfiles.cpp uses a TCP file upload client with I/O multiplexing;
    This program is for learning and sharing purposes and does not bear any legal liability;
    The program runs on Linux. tcppuutfiles.cpp is the program source code and does not include the development framework _public.h;
    The author will include a compiled executable file tcppuutfiles;
    Author: Zhao Zihan
*/

#include "_public.h"
using namespace idc; 

// 程序运行的参数结构体。
struct st_arg
{
    int  clienttype;                    // 客户端类型，1-上传文件；2-下载文件，本程序固定填1。
    char ip[31];                        // 服务端的IP地址。
    int  port;                          // 服务端的端口。
    char clientpath[256];               // 本地文件存放的根目录。 /data /data/aaa /data/bbb
    int  ptype;                         // 文件上传成功后本地文件的处理方式：1-删除文件；2-移动到备份目录。
    char clientpathbak[256];            // 文件成功上传后，本地文件备份的根目录，当ptype==2时有效。
    bool andchild;                      // 是否上传clientpath目录下各级子目录的文件，true-是；false-否。
    char matchname[256];                // 待上传文件名的匹配规则，如"*.TXT,*.XML"。
    char srvpath[256];                  // 服务端文件存放的根目录。/data1 /data1/aaa /data1/bbb
    int  timetvl;                       // 扫描本地目录文件的时间间隔（执行文件上传任务的时间间隔），单位：秒。 
    int  timeout;                       // 进程心跳的超时时间。
    char pname[51];                     // 进程名，建议用"tcpputfiles_后缀"的方式。
} starg;

class clogfile logfile;
class ctcpclient tcpcilent;

string strsendbuffer;   // 发送报文的buffer。
string strrecvbuffer;   // 接收报文的buffer。

void EXIT(int sig);     // 程序退出和信号2、15的处理函数。
bool activetest();      // 心跳。

void _help();                                               //帮助文档;
bool _xmlbuffer(const string &strxmlbuffer);                //解析xml参数;
bool login(const char *argv);                               //将客户端结构体数据传递给服务端;
bool _tcpputfiles(bool &bcontinue);                         //上传文件主函数;
bool ackmessage(const string& _strrecvbuffer);              //文件处理函数;
bool sendfile(const string &filename,const int filesize);   //文件传输函数;

int main(int argc,char *argv[]){

    if(argc != 3){ _help();return -1; }

    //处理信号; 关闭所有io; 
    //closeioandsignal(1);//本程序正式运行开启这段代码;
    signal(SIGINT,EXIT);
    signal(SIGINT,EXIT);

    //打开日志文件;
    if(logfile.open(argv[1])==false){
        cout<<"打开日志文件失败\n"; return -1;
    }else{
        logfile.write("打开日志文件成功\n");
    }
    
    //解析xml;
    if(_xmlbuffer(argv[2])==false){
        logfile.write("解析xml失败\n");
        return -1;
    }

    if(tcpcilent.connect(starg.ip,starg.port) ==false ){
        logfile<<"tcpcilent.connect()失败\n"; EXIT(-1);
    }else{
        logfile<<"tcpcilent.connect()成功\n"; 
    }

    //把参数传递给服务端;
    if(login(argv[2])==false){
        logfile.write("login()传递失败\n"); EXIT(-1);
    }

    bool bcontinue=true;

    while(1){

        if (_tcpputfiles(bcontinue) == false){
            logfile.write("_tcpputfiles() failed.\n"); EXIT(-1);
        }
        
        if(bcontinue == false){
           sleep(starg.timetvl);
        
            if(activetest()==false){
                break;
            }
        }

    }

    EXIT(0);

    return 0;

}




void EXIT(int sig){     // 程序退出和信号2、15的处理函数。
    logfile << "接收到信号 sig=" << sig << "\n";
    exit(0);
}

bool activetest(){     // 心跳。

    strsendbuffer.clear(); strrecvbuffer.clear();
    
    strsendbuffer="<activetest>ok</activetest>";
    if(tcpcilent.write(strsendbuffer)==false){
       //xxxxxxxxxxxxxxx logfile.write("心跳报文( %s )发送失败\n",strsendbuffer.c_str());
        return 0;
    }
    //xxxxxxxxxxxxxxx logfile.write("发送心跳报文:( %s )成功\n",strsendbuffer.c_str());

    if(tcpcilent.read(strrecvbuffer,60)==false){
        //xxxxxxxxxxxxxxxlogfile.write("心跳报文接收失败\n");
        return 0;
    }

    //xxxxxxxxxxxxxxx logfile.write("接收心跳报文:( %s )成功\n",strrecvbuffer.c_str());


    return 1;
}

//帮助文档;
void _help(){

    printf("\n<帮助文档>\n");
    printf("Using:/project/tools/bin/tcpputfiles logfilename xmlbuffer\n\n");

    printf("Sample:/home/zihan/zzhproject/tools/cpp/procctl 20 /home/zihan/zzhproject/tools/cpp/tcpputfiles "\
            " /log/idc/tcpputfiles_surfdata.log "\
            "\"<ip>192.168.218.129</ip><port>5005</port>"\
            "<clientpath>/tmp/client</clientpath><ptype>1</ptype>"
            "<srvpath>/tmp/server</srvpath>"\
            "<andchild>true</andchild><matchname>*.xml,*.txt,*.csv,*.json</matchname><timetvl>10</timetvl>"\
            "<timeout>50</timeout><pname>tcpputfiles_surfdata</pname>\"\n\n");

    printf("本程序是数据中心的公共功能模块，采用tcp协议把文件上传给服务端。\n");
    printf("logfilename   本程序运行的日志文件。\n");
    printf("xmlbuffer     本程序运行的参数，如下：\n");
    printf("ip            服务端的IP地址。\n");
    printf("port          服务端的端口。\n");
    printf("ptype         文件上传成功后的处理方式：1-删除文件；2-移动到备份目录。\n");
    printf("clientpath    本地文件存放的根目录。\n");
    printf("clientpathbak 文件成功上传后，本地文件备份的根目录，当ptype==2时有效。\n");
    printf("andchild      是否上传clientpath目录下各级子目录的文件，true-是；false-否，缺省为false。\n");
    printf("matchname     待上传文件名的匹配规则，如\"*.TXT,*.XML\"\n");
    printf("srvpath       服务端文件存放的根目录。\n");
    printf("timetvl       扫描本地目录文件的时间间隔，单位：秒，取值在1-30之间。\n");
    printf("timeout       本程序的超时时间，单位：秒，视文件大小和网络带宽而定，建议设置50以上。\n");
    printf("pname         进程名，尽可能采用易懂的、与其它进程不同的名称，方便故障排查。\n\n");
}

//解析xml;
bool _xmlbuffer(const string &strxmlbuffer){
    memset(&starg,0,sizeof(struct st_arg));

    getxmlbuffer(strxmlbuffer,"ip",starg.ip);
    if (strlen(starg.ip)==0) { logfile.write("ip is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"port",starg.port);
    if ( starg.port==0) { logfile.write("port is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"ptype",starg.ptype);
    if ((starg.ptype!=1)&&(starg.ptype!=2)) { logfile.write("ptype not in (1,2).\n"); return false; }

    getxmlbuffer(strxmlbuffer,"clientpath",starg.clientpath);
    if (strlen(starg.clientpath)==0) { logfile.write("clientpath is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"clientpathbak",starg.clientpathbak);
    if ((starg.ptype==2)&&(strlen(starg.clientpathbak)==0)) { logfile.write("clientpathbak is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"andchild",starg.andchild);

    getxmlbuffer(strxmlbuffer,"matchname",starg.matchname);
    if (strlen(starg.matchname)==0) { logfile.write("matchname is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"srvpath",starg.srvpath);
    if (strlen(starg.srvpath)==0) { logfile.write("srvpath is null.\n"); return false; }

    getxmlbuffer(strxmlbuffer,"timetvl",starg.timetvl);
    if (starg.timetvl==0) { logfile.write("timetvl is null.\n"); return false; }

    // 扫描本地目录文件的时间间隔（执行上传任务的时间间隔），单位：秒。
    // starg.timetvl没有必要超过30秒。
    if (starg.timetvl>30) starg.timetvl=30;

    // 进程心跳的超时时间，一定要大于starg.timetvl。
    getxmlbuffer(strxmlbuffer,"timeout",starg.timeout);
    if (starg.timeout==0) { logfile.write("timeout is null.\n"); return false; }
    if (starg.timeout<=starg.timetvl)  { logfile.write("starg.timeout(%d) <= starg.timetvl(%d).\n",starg.timeout,starg.timetvl); return false; }

    getxmlbuffer(strxmlbuffer,"pname",starg.pname,50);
    //if (strlen(starg.pname)==0) { logfile.write("pname is null.\n"); return false; }

   //logfile.write("srvpath=%s\n",starg.srvpath.c_str());

    return true;
}

//传递一整个xml;
bool login(const char *argv){

    strsendbuffer.clear();strrecvbuffer.clear();

    sformat(strsendbuffer,"<clienttype>1</clienttype>%s",argv);

    //xxxxxxxxxxxxxxx logfile.write("login函数 %s\n",strsendbuffer.c_str());

    if(tcpcilent.write(strsendbuffer)==false){
        logfile.write("login()发送strsendbuffer失败\n"); return 0;
    }else{
        //xxxxxxxxxxxxxxx logfile.write("login()发送strsendbuffer成功\n");
    }

    if(tcpcilent.read(strrecvbuffer,20)==false){
        logfile.write("login()strrecvbuffer接收失败\n"); return 0;
    }else{
        //xxxxxxxxxxxxxxx logfile.write("login()strrecvbuffer接收成功\n");
    }

    logfile.write("登录(%s:%d)成功。\n",starg.ip,starg.port); 
    
    return 1;
}


bool _tcpputfiles(bool &bcontinue){
    
    bcontinue=false;

    class cdir dir;
    strsendbuffer.clear();strrecvbuffer.clear();
    
    //打开目录;
    if(dir.opendir(starg.clientpath,starg.matchname,1000,starg.andchild)==false){
        logfile.write("_tcpputfiles函数 打目录失败\n");return false;
    }

    int delayed = 0; // 未收到对端确认报文的文件数量，发送了一个文件就加1，接收到了一个回应就减1。

    //获取目录文件名及其信息;
    while(dir.readdir()){
        
        bcontinue = true;
        
        //发送文件信息;
        sformat(strsendbuffer,"<filename>%s</filename><mtime>%s</mtime><size>%d</size>",
                dir.m_ffilename.c_str(),dir.m_mtime.c_str(),dir.m_filesize);

        if(tcpcilent.write(strsendbuffer)==false){
            logfile.write("_tcpputfiles函数 发送失败\n");return false;
        }
        //xxxxxxxxxxxxxxx logfile.write("_tcpputfiles函数 发送数据( %s )成功\n",strsendbuffer.c_str());
    
        //上传文件内容; 发送文件内容。
        logfile.write("send %s(%d) ...",dir.m_ffilename.c_str(),dir.m_filesize);
        if (sendfile(dir.m_ffilename,dir.m_filesize)==true)
        {
            logfile << "ok.\n"; delayed++;
        }
        else
        {
            logfile << "failed.\n"; tcpcilent.close(); return false;
        }
        
        //接收信息;
        while(delayed > 0){
            if(tcpcilent.read(strrecvbuffer,-1)==false){
                break;
            }
            //xxxxxxxxxxxxxxxxxxxxxxxxx logfile.write("_tcpputfiles函数 接收数据( %s )成功\n",strrecvbuffer.c_str());

            ackmessage(strrecvbuffer);  //处理接收信息;
        }
    }

    //接收信息;
    while(delayed > 0){
        if(tcpcilent.read(strrecvbuffer,10)==false){
           //xxxxxxxxxxxxxxxxxxxxxxxxx logfile.write("_tcpputfiles函数循环外 接收失败\n");
            break;
        }
        //xxxxxxxxxxxxxxx logfile.write("_tcpputfiles recv \n",strrecvbuffer.c_str());
        
        delayed--;
        ackmessage(strrecvbuffer);  //处理接收信息;
    }

    return 1;
}

//处理接收信息;
bool ackmessage(const string& _strrecvbuffer){

    //starg.ptype==1 删除;starg.ptype==2 备份; 
    //<filename>/tmp/client/2.txt</filename><return>ok</return>

    string filename;filename.clear();
    string result; result.clear();

    getxmlbuffer(_strrecvbuffer,"filename",filename);
    getxmlbuffer(_strrecvbuffer,"result",result);

    if(result != "ok"){ return true;}

    if(starg.ptype==1){
        if(remove(filename.c_str()) != 0 ){
            logfile.write("文件%s删除失败\n",filename.c_str());
            return 0;
        }else{
            //xxxxxxxxxxxxxxx logfile.write("文件%s删除成功\n",filename.c_str());
        }

    }

    if(starg.ptype == 2){
        string bakfilename = filename;
        replacestr(bakfilename,starg.clientpath,starg.clientpathbak,false);

        if(renamefile(filename,bakfilename) == false){
            logfile.write("renamefile(%s,%s) failed.\n",filename.c_str(),bakfilename.c_str());
            return false; 
        }
    }

    return true;

}

//传输文件;
bool sendfile(const string &filename,const int filesize){

    int onread=0;
    int totalbytes=0;
    char buffer[1001];
    class cifile ifile;

    if(ifile.open(filename,ios::in|ios::binary) == false){
        logfile.write("sendfile 函数打开文件失败;\n");
        return 0;
    }

    while(1){
        memset(buffer,0,sizeof(buffer));

        if(filesize-totalbytes > 1000){
            onread=1000;
        }else{
            onread=filesize-totalbytes;
        }

        ifile.read(buffer,onread);

        if(tcpcilent.write(buffer,onread) == false){
            logfile.write("sendfile 函数发送文件失败;\n");return false;
        }

        totalbytes=totalbytes+onread;
        
        if(totalbytes==filesize){
            break;
        }
    }

    return 1;
}