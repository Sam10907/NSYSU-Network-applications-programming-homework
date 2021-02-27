#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <chrono>
#include <future>
std::string GetLineFromCin() {
   std::string line;
   std::getline(std::cin, line);
   return line;
}
int main(void){
   //輸入連線資訊
   printf("[student @ CSE ~ ]$");
   char command[15]="";
   char ipaddr[20]="";
   int port;
   char name[50]="";
   scanf("%s%s%d%s",command,ipaddr,&port,name);
   //設定socket資訊
   int fd=socket(AF_INET,SOCK_STREAM,0);
   //將socket設為non-blocking
   int flags=fcntl(fd,F_GETFL,0);
   fcntl(fd,F_SETFL,flags | O_NONBLOCK);

   struct sockaddr_in srv;
   srv.sin_family=AF_INET;
   srv.sin_port=htons(port);
   srv.sin_addr.s_addr=inet_addr(ipaddr);
   while(1){
      int m=connect(fd,(struct sockaddr*) &srv,sizeof(srv));
      if(m == 0 && errno == EINPROGRESS) continue;
      else break;
   }
   //連線成功
   send(fd,name,strlen(name),0); //送名字
   std::cin.get(); //key point
   auto future = std::async(std::launch::async, GetLineFromCin);
   std::cout<<"開始聊天........"<<std::endl;
   while(1){
      char buf[500]=""; //接收訊息
      if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            auto line = future.get();
            // Set a new line. Subtle race condition between the previous line
            // and this. Some lines could be missed. To aleviate, you need an
            // io-only thread. I'll give an example of that as well.
            future = std::async(std::launch::async, GetLineFromCin);

            if(!strcmp(line.c_str(),"bye")){
               char buf1[200]="";
               int rbytes=recv(fd,buf1,sizeof(buf1),0);
               if(rbytes == 0){}
               else printf("%s",buf1);
               break;
            }
            send(fd,line.c_str(),strlen(line.c_str()),0);
      }
      int rbytes;
      rbytes=recv(fd,buf,sizeof(buf),0);
      if(rbytes == 0){}
      else printf("%s",buf);
      std::this_thread::sleep_for(std::chrono::seconds(1));
   }
   close(fd);
   return 0;
}