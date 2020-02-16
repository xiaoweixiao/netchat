#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<errno.h>

#define ERR_EXIT(x) do{perror(x);exit(EXIT_FAILURE);}while(0)

void client(char* ip,int port)
{
    int sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0)
        ERR_EXIT("socket");

    struct sockaddr_in peer;
    peer.sin_family = AF_INET;
    peer.sin_port = htons(port);
    peer.sin_addr.s_addr = inet_addr(ip);
    if(connect(sock,(struct sockaddr*)&peer,sizeof(peer)) < 0)
        ERR_EXIT("connect");

    int ret = -1;
    char buff[512] = {0};
    /*客户端主动去连接
      memset(buff,0,sizeof(buff));
      send(sock,buff,sizeof(buff),0);
      ret = recv(sock,buff,sizeof(buff),0);
      if(strcmp(buff,"n")==0)
      {
      printf("failure:%s %d\n",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
      return ;
      }
      else if(strcmp(buff,"y")==0)
      printf("success:%s %d\n",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
     */
    while(1)
    {
        memset(buff,0,sizeof(buff));
        printf("your message:\n");
        if(fgets(buff,sizeof(buff),stdin) == NULL)
            ERR_EXIT("fgets");
        send(sock,buff,strlen(buff),0);
        /*
           memset(buff,0,sizeof(buff));
           ret = recv(sock,buff,sizeof(buff),0);
           if(ret < 0)
           ERR_EXIT("recv");
           else if(ret > 0){
           char head[1024] = "sent msg:";
           strcat(head,buff);
           fputs(head,stdout);
           }*/
    }
}

void server(int port)
{
    int con = 0;
    int sock = socket(AF_INET,SOCK_DGRAM,0);
    if(sock < 0)
        ERR_EXIT("socket");

    struct sockaddr_in local;
    memset(&local,0,sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(port);
    local.sin_addr.s_addr = INADDR_ANY;
    if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
        ERR_EXIT("bind");
    int ret  = 0;
    char buff[512] = {0};
    struct sockaddr_in peer;
    socklen_t peerlen = sizeof(peer);

    /*连接过程
      memset(buff,0,sizeof(buff));
      ret = recvfrom(sock,buff,1024,0,(struct sockaddr*)&peer,&peerlen);
      if(ret < 0)
      ERR_EXIT("recvfrom");
      else if(ret > 0){
      int n = 3;
      while(n > 0){
      printf("session request from: %s %d\n",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
      sleep(5);
      n--;
      }

      printf("accept request?(y/n)");
      fgets(buff,1,stdin);
      if(strcmp(buff,"n")==0)
      {
      sendto(sock,buff,strlen(buff),0,(struct sockaddr*)&peer,peerlen);
      return ;
      }
      else if(strcmp(buff,"y")==0)
      sendto(sock,buff,strlen(buff),0,(struct sockaddr*)&peer,peerlen);
     */
    //进入通信循环
    while(1){
        memset(buff,0,sizeof(buff));
        ret = recvfrom(sock,buff,1024,0,(struct sockaddr*)&peer,&peerlen);
        if(ret < 0){
            if(errno == EINTR)//信号中断
                continue;
            ERR_EXIT("recvfrom");
        }
        else if(ret > 0){
            if(con == 0){
                con = 1;
                for(int i=0;i<3;i++){
                    printf("session request from: %s %d\n",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
                    printf("chat?:\n");
                    sleep(20);
                }
                /*
                if(out_time == 1)
      printf("success:%s %d\n",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
                else
      printf("failure:%s %d\n",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
*/            }
            else{
      printf("[%s %d]",inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
                char head[1024] = "sent msg:";
                strcat(head,buff);
                fputs(head,stdout);
            }

            /*
               printf("your message:\n");
               memset(buff,0,sizeof(buff));
               fgets(buff,sizeof(buff),stdin);
               sendto(sock,buff,strlen(buff),0,(struct sockaddr*)&peer,peerlen);*/
        }
    }
    close(sock);
}
int main(int argc,char* argv[])
{
    if(argc != 2)
    {
        ERR_EXIT("Usage:netchat port");
    }

    pid_t pro = fork();
    if(pro < 0 )
        ERR_EXIT("fork");
    else if(pro == 0)
    {
        printf("chat?:\n");
        char ip[24] = {0};
        int port = 0;
        scanf("%s%d",ip,&port);
        client(ip,port);
    }
    else
    {
        //父进程
        server(atoi(argv[1]));
    }
    return 0;
}
