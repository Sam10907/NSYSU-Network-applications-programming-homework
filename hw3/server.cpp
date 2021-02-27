#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include <vector>
#include <time.h>
#include "str_handle.cpp"
using namespace std;
map<int,string> socket_to_ipaddr;
map<string,string> name_to_ipaddr;
map<int,string> socket_to_name;
string Users[3]={"Alice","Bill","Caesar"}; //存在的使用者
int user_notify[3]={0};
vector<string> off_line_text; //存放離線訊息
vector<string> off_line_user; //存放離線的使用者
char *str_time;
int getMessage(char**,char*,char*,const char*); //處理獲取的訊息
string find_name(int);
int main(void){
    //建立socket資訊
    int fd=socket(AF_INET,SOCK_STREAM,0);
    //將socket設為non-blocking
    int flags=fcntl(fd,F_GETFL,0);
    fcntl(fd,F_SETFL,flags | O_NONBLOCK);

    struct sockaddr_in srv,cli;
    socklen_t clilen=sizeof(cli);
    bzero(&srv , sizeof(srv));
    srv.sin_family=AF_INET;
    srv.sin_port=htons(1234);
    srv.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(fd,(struct sockaddr*) &srv,sizeof(srv)) <0 || listen(fd,5)<0){
        perror("error!\n");
        exit(1);
    }else{
        fd_set rfds;
        fd_set afds;
        int maxfd=fd;
        FD_ZERO(&afds);
        FD_ZERO(&rfds);
        FD_SET(fd,&afds);
        int newfd,a;
        while(1){
            memcpy(&rfds,&afds,sizeof(rfds));
            if((a=select(maxfd+1,&rfds,NULL,NULL,NULL)) < 0){
                perror("no ready read file descriptor\n");
                exit(1);
            }
            //printf("%d\n",a);
            if(FD_ISSET(fd,&rfds)){ //如果有連線請求
                //printf("yes1\n");
                newfd=accept(fd,(struct sockaddr*)&cli,&clilen);
                if(newfd == -1){ 
                    printf("accept error.\n");
                    exit(1); 
                }
                else{
                    //將newfd設為non-blocking
                    int flags=fcntl(newfd,F_GETFL,0);
                    fcntl(newfd,F_SETFL,flags | O_NONBLOCK);
                    maxfd=newfd;
                    string ipaddr(inet_ntoa(cli.sin_addr));
                    socket_to_ipaddr.insert(pair<int,string>(newfd,ipaddr));
                    FD_SET(newfd,&afds);
                }
            }
            for(int i=0;i<maxfd+1;i++){
                if(i != fd && FD_ISSET(i,&rfds)){
                    //printf("enter\n");
                    map<int,string>::iterator iter;
                    if((iter=socket_to_ipaddr.find(i)) != socket_to_ipaddr.end()){
                        //printf("enter1\n");
                        char message_buf[200]="";
                        int rbytes=recv(iter->first,message_buf,sizeof(message_buf),0); //receive message
                        if(rbytes == 0){  //User client離線
                            //printf("0data\n");
                            FD_CLR(i,&afds);
                            //傳送下線訊息
                            char message[200]="";
                            strcat(message,"<User ");
                            strcat(message,socket_to_name.find(i)->second.c_str());
                            strcat(message," is off-line.>\n");
                            for(map<int,string>::iterator itr=socket_to_ipaddr.begin();itr!=socket_to_ipaddr.end();itr++){
                                if(itr->first == i) continue;
                                send(itr->first,message,strlen(message),0);
                            }
                            for(int j=0;j<3;j++){ //將上線通知次數歸零
                                if(socket_to_name.find(i)->second == Users[j]) user_notify[j]=0;
                            }
                            //清除上線資訊
                            socket_to_ipaddr.erase(i);
                            map<int,string>::iterator user_name=socket_to_name.find(i);
                            name_to_ipaddr.erase(user_name->second);
                            socket_to_name.erase(i);
                        }
                        else{ //處理對話
                            char **argv=strtok_command(message_buf," ");
                            if(argv[1] == (char*)0){
                                printf("%s\n",message_buf);
                                string Name(message_buf);
                                name_to_ipaddr.insert(pair<string,string>(Name,iter->second));
                                socket_to_name.insert(pair<int,string>(i,Name));
                                for(int j=0;j<off_line_user.size();j++){ //傳送離線訊息
                                    if(off_line_user[j] == Name){
                                        const char *msg=off_line_text[j].c_str();
                                        char **argv=strtok_command(msg,":");
                                        char off_line_message[250]="<User ";
                                        strcat(off_line_message,argv[0]);
                                        strcat(off_line_message," has sent you a message \"");
                                        strncat(off_line_message,argv[1],strlen(argv[1])-1);
                                        strcat(off_line_message,"\" at ");
                                        strncat(off_line_message,str_time,strlen(str_time)-1);
                                        strcat(off_line_message,">\n");
                                        send(i,off_line_message,strlen(off_line_message),0);
                                        off_line_user.erase(off_line_user.begin()+j);
                                        off_line_text.erase(off_line_text.begin()+j);
                                        j--;
                                    }
                                }
                            }
                            //傳送上線訊息
                            if(user_notify[0] == 0 || user_notify[1] == 0 || user_notify[2] == 0){ //使每位使用者被通知一次
                                char message[350]="";
                                bool isSend=false;
                                for(map<string,string>::iterator itr=name_to_ipaddr.begin();itr!=name_to_ipaddr.end();itr++){
                                    if(socket_to_name.find(i)->second == itr->first) continue;
                                    strcat(message,"<User ");
                                    strcat(message,itr->first.c_str());
                                    strcat(message," is on-line, IP address: ");
                                    strcat(message,itr->second.c_str());
                                    strcat(message,".>\n");
                                    isSend=true;
                                }
                                if(isSend){
                                    for(int j=0;j<3;j++){
                                        if(socket_to_name.find(i)->second == Users[j]){
                                            if(user_notify[j] == 0){
                                                send(i,message,strlen(message),0);
                                                user_notify[j]++;
                                            }
                                            break;
                                        }
                                    }
                                }
                            }

                            char message[200]="";
                            string str_name=find_name(i);
                            const char *name=str_name.c_str();
                            int num=getMessage(argv,message,message_buf,name); //判斷單播或多播的依據
                            printf("%d %s",num,message);
                            if(num-1 == 1){ //單播
                                string UserName(argv[1]);
                                bool isUser=false;
                                for(int i1=0;i1<3;i1++){ //判斷是否是存在的使用者
                                    if(UserName == Users[i1]){
                                        isUser=true;
                                        break;
                                    }
                                }
                                if(isUser){
                                    map<string,string>::iterator iter1;
                                    map<int,string>::iterator s_iter;
                                    if((iter1=name_to_ipaddr.find(UserName)) != name_to_ipaddr.end()){ //User上線
                                        for(s_iter=socket_to_name.begin();s_iter!=socket_to_name.end();s_iter++){
                                            if(s_iter->second == UserName){
                                                int sbytes=send(s_iter->first,message,strlen(message),0);
                                                if(sbytes == -1){ //當傳送過程中對方離線時
                                                    time_t t = time(NULL);
                                                    str_time=ctime(&t);
                                                    string str_msg(message);
                                                    off_line_text.push_back(str_msg);
                                                    off_line_user.push_back(UserName);
                                                    char user_off_line_msg[100]="<User ";
                                                    strcat(user_off_line_msg,UserName.c_str());
                                                    strcat(user_off_line_msg," is off-line. The message will be passed when he come back.>\n");
                                                    send(i,user_off_line_msg,strlen(user_off_line_msg),0);
                                                }
                                                break;
                                            }
                                        }
                                    }else{ //User離線
                                            time_t t = time(NULL);
                                            str_time=ctime(&t);
                                            string str_msg(message);
                                            off_line_text.push_back(str_msg);
                                            off_line_user.push_back(UserName);
                                            char user_off_line_msg[100]="<User ";
                                            strcat(user_off_line_msg,UserName.c_str());
                                            strcat(user_off_line_msg," is off-line. The message will be passed when he come back.>\n");
                                            send(i,user_off_line_msg,strlen(user_off_line_msg),0);
                                    }
                                }else{ //不存在的使用者
                                    char error_msg[100]="<User ";
                                    strcat(error_msg,UserName.c_str());
                                    strcat(error_msg," does not exist.>\n");
                                    send(i,error_msg,strlen(error_msg),0);
                                }
                            }else if(num-1 == 2){ //多播
                                string UserName(argv[1]);
                                string UserName1(argv[2]);
                                int isUser=0,exist=-1;
                                for(int i1=0;i1<3;i1++){ //判斷是否是存在的使用者1
                                    if(UserName == Users[i1]){
                                        isUser++;
                                        exist=i1;
                                        break;
                                    }
                                }
                                for(int i1=0;i1<3;i1++){ //判斷是否是存在的使用者2
                                    if(UserName1 == Users[i1]){
                                        isUser++;
                                        exist=i1;
                                        break;
                                    }
                                }
                                map<string,string>::iterator iter1,iter2;
                                iter2=name_to_ipaddr.find(UserName1);
                                map<int,string>::iterator s_iter;
                                if(isUser == 2){ //當兩個使用者都存在時
                                    if((iter1=name_to_ipaddr.find(UserName)) != name_to_ipaddr.end() && iter2 != name_to_ipaddr.end()){ //2 User上線
                                        for(s_iter=socket_to_name.begin();s_iter!=socket_to_name.end();s_iter++){ //傳給使用者1
                                            if(s_iter->second == UserName){
                                                int sbytes=send(s_iter->first,message,strlen(message),0);
                                                if(sbytes == -1){ //當傳送過程中對方離線時
                                                    time_t t = time(NULL);
                                                    str_time=ctime(&t);
                                                    string str_msg(message);
                                                    off_line_text.push_back(str_msg);
                                                    off_line_user.push_back(UserName);
                                                    char user_off_line_msg[100]="<User ";
                                                    strcat(user_off_line_msg,UserName.c_str());
                                                    strcat(user_off_line_msg," is off-line. The message will be passed when he come back.>\n");
                                                    send(i,user_off_line_msg,strlen(user_off_line_msg),0);
                                                }
                                                break;
                                            }
                                        }
                                        for(s_iter=socket_to_name.begin();s_iter!=socket_to_name.end();s_iter++){ //傳給使用者2
                                            if(s_iter->second == UserName1){
                                                int sbytes=send(s_iter->first,message,strlen(message),0);
                                                if(sbytes == -1){ //當傳送過程中對方離線時
                                                    time_t t = time(NULL);
                                                    str_time=ctime(&t);
                                                    string str_msg(message);
                                                    off_line_text.push_back(str_msg);
                                                    off_line_user.push_back(UserName);
                                                    char user_off_line_msg[100]="<User ";
                                                    strcat(user_off_line_msg,UserName.c_str());
                                                    strcat(user_off_line_msg," is off-line. The message will be passed when he come back.>\n");
                                                    send(i,user_off_line_msg,strlen(user_off_line_msg),0);
                                                }
                                                break;
                                            }
                                        }
                                    }else{ //1 User離線或2 Users離線
                                        if(iter1 == name_to_ipaddr.end() && iter2 != name_to_ipaddr.end()){ //User1離線
                                            time_t t = time(NULL);
                                            str_time=ctime(&t);
                                            string str_msg(message);
                                            off_line_text.push_back(str_msg);
                                            off_line_user.push_back(argv[1]);
                                            char user_off_line_msg[100]="<User ";
                                            strcat(user_off_line_msg,argv[1]);
                                            strcat(user_off_line_msg," is off-line. The message will be passed when he come back.>\n");
                                            send(i,user_off_line_msg,strlen(user_off_line_msg),0);
                                            for(map<int,string>::iterator itr=socket_to_name.begin();itr!=socket_to_name.end();itr++){ //送user2訊息
                                                if(itr->second == argv[2]) send(itr->first,message,strlen(message),0);
                                            }
                                        }
                                        else if(iter2 == name_to_ipaddr.end() && iter1 != name_to_ipaddr.end()){ //User2 離線
                                            time_t t = time(NULL);
                                            str_time=ctime(&t);
                                            string str_msg(message);
                                            off_line_text.push_back(str_msg);
                                            off_line_user.push_back(argv[2]);
                                            char user_off_line_msg[100]="<User ";
                                            strcat(user_off_line_msg,argv[2]);
                                            strcat(user_off_line_msg," is off-line. The message will be passed when he come back.>\n");
                                            send(i,user_off_line_msg,strlen(user_off_line_msg),0);
                                            for(map<int,string>::iterator itr=socket_to_name.begin();itr!=socket_to_name.end();itr++){ //送user1訊息
                                                if(itr->second == argv[1]) send(itr->first,message,strlen(message),0);
                                            }
                                        }else{ //兩個Users都離線
                                            time_t t = time(NULL);
                                            str_time=ctime(&t);
                                            string str_msg(message);
                                            off_line_text.push_back(str_msg);
                                            off_line_user.push_back(argv[1]);
                                            off_line_text.push_back(str_msg);
                                            off_line_user.push_back(argv[2]);
                                            char user_off_line_msg[200]="<User ";
                                            strcat(user_off_line_msg,argv[1]);
                                            strcat(user_off_line_msg," ");
                                            strcat(user_off_line_msg,argv[2]);
                                            strcat(user_off_line_msg," are off-line. The message will be passed when they come back.>\n");
                                            send(i,user_off_line_msg,strlen(user_off_line_msg),0);
                                        }
                                    }
                                }else{ //其中有一個或兩個不存在的使用者
                                    if(exist != -1){
                                        if(Users[exist] == UserName){
                                            char error_msg[100]="<User ";
                                            strcat(error_msg,UserName1.c_str());
                                            strcat(error_msg," does not exist.>\n");
                                            send(i,error_msg,strlen(error_msg),0);
                                            bool isoff_line=true;
                                            for(map<int,string>::iterator itr=socket_to_name.begin();itr!=socket_to_name.end();itr++){
                                                if(itr->second == UserName){ //上線時送訊息
                                                    isoff_line=false;
                                                    send(itr->first,message,strlen(message),0);
                                                    break;
                                                }
                                            }
                                            if(isoff_line){ //離線
                                                time_t t = time(NULL);
                                                str_time=ctime(&t);
                                                string str_msg(message);
                                                off_line_text.push_back(str_msg);
                                                off_line_user.push_back(UserName);
                                                char user_off_line_msg[100]="<User ";
                                                strcat(user_off_line_msg,UserName.c_str());
                                                strcat(user_off_line_msg," is off-line. The message will be passed when he come back.>\n");
                                                send(i,user_off_line_msg,strlen(user_off_line_msg),0);
                                            }
                                        }
                                        else if(Users[exist] == UserName1){
                                            char error_msg[100]="<User ";
                                            strcat(error_msg,UserName.c_str());
                                            strcat(error_msg," does not exist.>\n");
                                            send(i,error_msg,strlen(error_msg),0);
                                            bool isoff_line=true;
                                            for(map<int,string>::iterator itr=socket_to_name.begin();itr!=socket_to_name.end();itr++){
                                                if(itr->second == UserName1){ //上線時送訊息
                                                    isoff_line=false;
                                                    send(itr->first,message,strlen(message),0);
                                                    break;
                                                }
                                            }
                                            if(isoff_line){ //離線
                                                time_t t = time(NULL);
                                                str_time=ctime(&t);
                                                string str_msg(message);
                                                off_line_text.push_back(str_msg);
                                                off_line_user.push_back(UserName1);
                                                char user_off_line_msg[100]="<User ";
                                                strcat(user_off_line_msg,UserName1.c_str());
                                                strcat(user_off_line_msg," is off-line. The message will be passed when he come back.>\n");
                                                send(i,user_off_line_msg,strlen(user_off_line_msg),0);
                                            }
                                        }
                                    }else{ //兩個使用者皆不存在
                                        char error_msg[100]="<User ";
                                        strcat(error_msg,UserName.c_str());
                                        strcat(error_msg," ");
                                        strcat(error_msg,UserName1.c_str());
                                        strcat(error_msg," does not exist.>\n");
                                        send(i,error_msg,strlen(error_msg),0);
                                    }
                                }
                            }   
                        }
                    }
                }
            }
        }
    }
}
int getMessage(char **cmd,char *message,char *msg_buf,const char *name){
    int num=0;
    int Mnum,i=0,i1=0;
    bool is_cat=false;
    while(cmd[num] != (char*)0){
        if(cmd[num][0] == '"') Mnum=num;
        num++;
    }
    char buf[100]="";
    while(*(msg_buf+i)!='\0'){
        if(is_cat){
            if(*(msg_buf+i) == '"') break;
            buf[i1]=*(msg_buf+i);
            i1++;
        }
        if(*(msg_buf+i) == '"') is_cat=true;
        i++;
    }
    buf[strlen(buf)]='\n';
    strcpy(message,name);
    strcat(message,":");
    strcat(message,buf);
    return Mnum;
}
string find_name(int fd){
    map<int,string>::iterator iter;
    iter=socket_to_name.find(fd);
    if(iter == socket_to_name.end()){
        printf("find no name\n");
        return "error";
    }
    string t=iter->second;
    return t;
}