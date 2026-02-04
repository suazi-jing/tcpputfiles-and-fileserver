/*
    fileserver.cpp 为文件上传服务端
    该程序用于学习分享,不承担任何法律风险;
    程序运行在Linux上, fileserver.cpp 是程序源码,不包含开发框架_public.h;
    作者会附带编译后可执行文件 fileserver
    作者:赵梓涵

    fileserver.cpp is the server for file uploads.  
    This program is for learning and sharing purposes and does not assume any legal liability;  
    The program runs on Linux. fileserver.cpp is the program source code and does not include the development framework _public.h;  
    The author will also provide the compiled executable file fileserver.  
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


void FathEXIT(int sig);     //父进程;
void ChldEXIT(int sig);     //子进程;

class clogfile logfile;
class ctcpserver fileserver;

bool clientlogin();                                                                     //处理客户端发来的数据;
void recvfilesmain();                                                                   // 上传文件的主函数。
bool recvfile(const string &filename,const string &filetime,const int filesize);        //接收文件内容函数;

string strsendbuffer;   // 发送报文的buffer。
string strrecvbuffer;   // 接收报文的buffer。

int main(int argc,char *argv[]){
    
    if(argc != 3){
        cout<<"\n<帮助文档>\n";
        cout<<"本程序 日志文件 通信端口\n";
        cout<<"/home/zihan/zzhproject/tools/cpp/fileserver /log/idc/fileserver.log 5005\n\n";
        return -1;
    }
    
    //处理信号; 关闭所有io; 
    //closeioandsignal(1);//本程序正式运行开启这段代码;
    signal(SIGINT,FathEXIT);
    signal(SIGINT,FathEXIT);
    
    //打开日志文件;
    if(logfile.open(argv[1])==false){
        cout<<"打开日志文件失败\n"; return -1;
    }else{
        logfile.write("打开日志文件成功\n");
    }

    // 服务端初始化。
    if(fileserver.initserver( atoi( argv[2] ) )==false){
        logfile<<"服务端初始化失败\n";return -1;
    }else{
        logfile<<"服务端初始化成功\n";
    }

    while(1){

        if(fileserver.accept()==false){
            logfile<<"server accept 失败.\n";FathEXIT(-1);
        }else{
            logfile<<"server accept 成功.\n";
        }
        
        //fork 子进程去完成客户端业务; 父进程只做监听;
        if(fork()>0){
            fileserver.closeclient(); continue;
        }

        fileserver.closelisten();
        
        signal(SIGINT,ChldEXIT); signal(SIGTERM,ChldEXIT);

        //处理客户端发来的数据;
        if(clientlogin()==false){ ChldEXIT(-1);}
        
        if(starg.clienttype == 1){
            recvfilesmain();  //上传业务主函数;
        }
        
        //if(){
        //  endfilesmain(); //备份业务主函数;
        //}

        ChldEXIT(0);

    }

    return 0;
}


void FathEXIT(int sig){     //父进程;
    
    //忽略2,15信号避免被干扰
    signal(2,SIG_IGN); signal(15,SIG_IGN);
    
    logfile << "父进程" << getpid() << "退出，sig=" << sig << "\n";

    //向子进程发送信号;
    kill(0,15);
    fileserver.closelisten();

    //父进程退出;
    exit(0);

}

void ChldEXIT(int sig){     //子进程;

    // 以下代码是为了防止信号处理函数在执行的过程中再次被信号中断。
    signal(SIGINT,SIG_IGN); signal(SIGTERM,SIG_IGN);

    logfile << "子进程" << getpid() << "退出，sig=" << sig << "\n";

    // 在这里增加释放资源的代码（只释放子进程的资源）。
    fileserver.closeclient();       // 子进程关闭客户端连上来的socket。
    
    exit(0);
}

void recvfilesmain(){   // 上传文件的主函数。

    strsendbuffer.clear(); strrecvbuffer.clear();

    while(1){
        if(fileserver.read(strrecvbuffer,60)==false){
            logfile.write("接收失败\n");
            return;
        }

       //xxxxxxxxxxxxxxxxxxxxxxxxx logfile.write("接收客户端报文:( %s )成功\n",strrecvbuffer.c_str());
        
        
        //发送心跳报文;
        if(strrecvbuffer=="<activetest>ok</activetest>"){
            
            strsendbuffer="ok";
            //xxxxxxxxxxxxxxxxxxxxxxxxx logfile.write("发送心跳报文:( %s )成功\n",strsendbuffer.c_str());

            if(fileserver.write(strsendbuffer)==false){
                logfile.write("发送失败\n");
                return;
            }
        }

        //发送上传文件回应报文;
        if(strrecvbuffer.find("filename") != string::npos ){
            string  filename; filename.clear();
            string  filetime; filetime.clear();
            int     filesize=0;

            getxmlbuffer(strrecvbuffer,"filename",filename);
            getxmlbuffer(strrecvbuffer,"mtime",filetime);
            getxmlbuffer(strrecvbuffer,"size",filesize);

            //接收文件内容;
            //拼接服务端文件名;
            string serverfilename = filename;
            replacestr(serverfilename,starg.clientpath,starg.srvpath,false);
            
            //xxxxxxxxxxxxxxxxxxx cout<<"serverfilename= "<<serverfilename<<" starg.clientpath="<<starg.clientpath<<" starg.srvpath="<<starg.srvpath<<endl;
            
            // 发送文件内容。
            logfile.write("recv %s(%d) ...",filename.c_str(),filesize);
            if (recvfile(serverfilename,filetime,filesize) == true)
            {
                logfile << "ok.\n";
                sformat(strsendbuffer,"<filename>%s</filename><result>ok</result>",filename.c_str()); 
            }
            else
            {
                logfile << "failed.\n";
                sformat(strsendbuffer,"<filename>%s</filename><result>failed</result>",filename.c_str());
            }

            if(fileserver.write(strsendbuffer)==false){
                logfile.write("发送回应文件信息报文失败\n");
                return;
            }else{
                //xxxxxxxxxxxxxxxxxxxxxxxxx  logfile.write("发送回应文件信息报文:( %s )成功\n",strsendbuffer.c_str());
            }

        }

    }
}

bool clientlogin(){     //处理客户端发来的数据;
    
    strsendbuffer.clear();strrecvbuffer.clear();
    
    //接收数据
    if(fileserver.read(strrecvbuffer,20)==false){
        logfile.write("接收数据失败\n"); return 0;
    }else{
        //xxxxxxxxxxxxxxxxxxxxxxxxx logfile.write("接收数据( %s )成功\n",strrecvbuffer.c_str()); 
    }

    //解析数据
    memset(&starg,0,sizeof(struct st_arg));
    getxmlbuffer(strrecvbuffer,"clienttype",starg.clienttype);
    getxmlbuffer(strrecvbuffer,"clientpath",starg.clientpath);
    getxmlbuffer(strrecvbuffer,"srvpath",starg.srvpath);

    //xxxxxxxxxxxxxxxxxxx cout<<"clientlogin函数"<<"starg.clienttype="<<starg.clienttype<<" starg.clientpath="<<starg.clientpath<<" starg.srvpath="<<starg.srvpath<<endl;

    //发送回应数据;
    if(starg.clienttype!=1 && starg.clienttype!=2 ){
        strsendbuffer="Error";
    }else{
        strsendbuffer="Ok";
    }

    if(fileserver.write(strsendbuffer)==false){
        logfile.write("发送数据失败\n");
        return 0;
    }else{
        //xxxxxxxxxxxxxxxxxxxxxxxxx logfile.write("发送数据成功\n"); 
    }

    return 1;
}


bool recvfile(const string &filename,const string &filetime,const int filesize){
    
    int  totalbytes=0;          // 已接收文件的总字节数。
    int  onread=0;              // 本次打算接收的字节数。
    char buffer[1000];          // 接收文件内容的缓冲区。
    cofile ofile;               // 写入文件的对象。
    
    //xxxxxxxxxxxxxxxxxxx cout<<"filename="<<filename<<" filetime="<<filetime<<" filesize="<<filesize<<endl;

    if (ofile.open(filename,true,ios::out|ios::binary)==false) return false;
   
    while(1){
        memset(buffer,0,sizeof(buffer));

         if(filesize-totalbytes > 1000){
            onread=1000;
        }else{
            onread=filesize-totalbytes;
        }

        if(fileserver.read(buffer,onread) == false){
            logfile.write("recvfile 函数接收文件失败;\n");
            return false;
        }

        if(ofile.write(buffer,onread)==false){
            logfile.write("ofile.write(buffer,onread)==false");
            return false;

        }

        totalbytes=totalbytes+onread;
        
        if(totalbytes==filesize){
            break;
        }
   }
    ofile.closeandrename();

    // 文件时间用当前时间没有意义，应该与对端的文件时间保持一致。
    setmtime(filename,filetime);
    return 1;
}







