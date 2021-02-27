#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include "huffman.h"
using namespace std;
int main(void){
    //輸入連線資訊
    printf("[student @ CSE ~ ]$");
    char command[15]="";
    char ipaddr[20]="";
    int port;
    scanf("%s%s%d",command,ipaddr,&port);
    //設定socket資訊
    int fd=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in srv;
    srv.sin_family=AF_INET;
    srv.sin_port=htons(port);
    srv.sin_addr.s_addr=inet_addr(ipaddr);
    if(connect(fd,(struct sockaddr*) &srv,sizeof(srv)) < 0){ //連線失敗
        perror("connect error\n");
        exit(1);
    }else{ //連線成功
        printf("The server with IP %s has accepted your connection.\n",ipaddr);
        printf("[student @ CSE ~ ]$");
        char file[50]; //檔名
        scanf("%s",command);
        while(strcmp(command,"goodbye")!=0){
            scanf("%s",file);
            send(fd,file,strlen(file),0); //傳送檔案名稱
            //關於壓縮檔的資訊
            long long original_file_length, compress_file_length;
            float compress_ratio;
            //壓縮檔案然後傳送
            char *compressed_file=compress_file(file,original_file_length,compress_file_length,compress_ratio);
            printf("Original file length: %lld bytes, compressed file length: %lld bytes (ratio: %f%%)\n",original_file_length,compress_file_length,(compress_ratio*100));
            printf("Using variable-length codeword\n");
            long long *ori=&original_file_length;
            long long *com=&compress_file_length;
            send(fd,ori,sizeof(long long),0); //傳送原始檔案的位元組大小
            send(fd,com,sizeof(long long),0); //傳送壓縮檔案大小
            FILE *fp;
            fp=fopen(compressed_file,"rb");
            while(!feof(fp)){  //一 一 傳送壓縮檔案資訊
                char *buffer=(char*) malloc(1024);
                int fread_nbytes=fread(buffer,sizeof(char),sizeof(buffer),fp); //實際讀取到多少size
                //write data to socket
                int nbytes;
                char *ptr=buffer;
                if((nbytes=send(fd,ptr,fread_nbytes,0)) < 0){ //傳送失敗
                    perror("send error\n");
                    exit(1);
                }else{ //傳送成功    下面的寫法是保證全數送出
                    int send_bytes=nbytes; //number of bytes  sended
                    nbytes=fread_nbytes-send_bytes; //left bytes number
                    ptr+=send_bytes; //剩餘未送的資料
                    while (nbytes){
                        send_bytes=send(fd,ptr,nbytes,0);
                        ptr+=send_bytes;
                        nbytes-=send_bytes;
                    }
                }
            }
            fclose(fp);
            printf("[student @ CSE ~ ]$");
            scanf("%s",command);
        }
    }
    printf("See you next time.\n");
    shutdown(fd,SHUT_RDWR);
}