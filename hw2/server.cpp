#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include "huffman.h"
int main(void){
    //建立socket資訊
    int fd=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in srv,cli;
    socklen_t clilen=sizeof(cli);
    srv.sin_family=AF_INET;
    srv.sin_port=htons(1234);
    srv.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(fd,(struct sockaddr*) &srv,sizeof(srv)) < 0){
        perror("bind error\n");
        exit(1);
    }else{ //開始監聽
        if(listen(fd,5) < 0){
            perror("listen error\n");
            exit(1);
        }else{
            int newfd=accept(fd,(struct sockaddr*) &cli,&clilen); //連線成功時會回傳新的socket file descriptor
            if(newfd < 0){ //連線失敗
                perror("accept error\n");
                exit(1);
            }else{  //連線成功
                printf("A client  \"%s\" has connected via port num %d using SOCK_STREAM(TCP)\n",inet_ntoa(cli.sin_addr),(int)ntohs(cli.sin_port));
                int isclose=-1; //client端是否關閉連線
                while(isclose!=0){
                    char file[50]=""; //檔名
                    long long o=0,c=0;
                    long long *ori=&o; //原始檔案大小
                    long long *com=&c; //壓縮檔案大小
                    isclose=recv(newfd,file,50,0); //接收檔名
                    if(isclose==0) continue;
                    recv(newfd,ori,sizeof(long long),0); //接收原始檔案大小
                    recv(newfd,com,sizeof(long long),0); //接收壓縮檔案大小
                    char sub_file[]=".";
                    char **argv=strtok_filename(sub_file,file); //將檔名與副檔名做切割
                    printf("The client sends a file \"%s\" with size of %lld bytes. The Huffman coding data are stored in \"%s_code.txt\".\n",file,o,argv[0]);
                    char *subfile=strcat(argv[0],".compress"); //將檔名加上壓縮檔的副檔名
                    FILE *fp;
                    fp=fopen(subfile,"wb");
                    int nbytes;
                    int sum=0;
                    char buffer[1024];
                    while(sum<*com){ //判斷是否已將檔案傳輸完畢
                        if((nbytes=recv(newfd,buffer,1024,0)) < 0){
                            perror("receive error\n");
                            exit(1);
                        }
                        else{
                            sum+=nbytes;
                            fwrite(buffer,sizeof(char),nbytes,fp); //寫入檔案
                        }
                    }
                    fclose(fp);
                    uncompress_file(subfile,file); //解壓縮
                }
                printf("The client \"%s\" with port %d has terminated the connection.\n",inet_ntoa(cli.sin_addr),(int)ntohs(cli.sin_port));
            }
            shutdown(newfd,SHUT_RDWR);
            shutdown(fd,SHUT_RDWR);
        }
    }
}